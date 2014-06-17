#include <string>
#define _ERROR_WINDOWS
#include "error.h"

#include "volume.h"
#include "utils.h"
#include "ntfs.h"
#include "ntfs_file.h"

#define NTFS_FMT_ERR MsgError(L"NTFS data structure parsing problem")
#define NTFS_FILE_REC_HEADER_SIZE offsetof(NTFS_FILE_RECORD_OUTPUT_BUFFER, FileRecordBuffer)
#define MAX_ATTR_LIST_SIZE (10 * 1024 * 1024)
#define CHECK_FMT(code) { if (!(code)) FAIL(NTFS_FMT_ERR); }

std::string AttrInfo::type_name( ) const {
	std::string type_name;
	if ( type >= AT_FIRST_USER_DEFINED_ATTRIBUTE ) type_name = L"USER_DEFINED";
	else {
		for ( unsigned i = 0; i < sizeof( c_attr_types ) / sizeof( c_attr_types[ 0 ] ); i++ ) {
			if ( c_attr_types[ i ].type == type ) {
				type_name = c_attr_types[ i ].name;
				break;
				}
			}
		}
	return type_name;
	}

// determine full file paths for FILE_NAME attributes
void FileInfo::find_full_paths( ) {
	unsigned fn_idx = 0;
	for ( unsigned attr_idx = 0; attr_idx < attr_list.size( ); attr_idx++ ) {
		if ( attr_list[ attr_idx ].type != AT_FILE_NAME ) {
			continue;
			}
		if ( file_name_list[ fn_idx ].name == L"." ) {
			attr_list.item( attr_idx ).name = add_trailing_slash( volume->name );
			}
		else {
			std::string full_path = file_name_list[ fn_idx ].name;
			u64 parent_dir_ref = file_name_list[ fn_idx ].parent_directory;
			while ( true ) {
				FileInfo file_info;
				file_info.volume = volume;
				file_info.process_file( parent_dir_ref );
				unsigned posix_name_idx = -1;
				unsigned win32_name_idx = -1;
				for ( unsigned i = 0; i < file_info.file_name_list.size( ); i++ ) {
					const FileNameAttr& attr = file_info.file_name_list[ i ];
					if ( attr.file_name_type == FILE_NAME_POSIX ) {
						posix_name_idx = i;
						}
					else if ( ( attr.file_name_type == FILE_NAME_WIN32 ) || ( attr.file_name_type == FILE_NAME_WIN32_AND_DOS ) ) {
						win32_name_idx = i;
						}
					}
				unsigned name_idx;
				if ( posix_name_idx != -1 ) {
					name_idx = posix_name_idx;
					}
				else if ( win32_name_idx != -1 ) {
					name_idx = win32_name_idx;
					}
				else {
					CHECK_FMT( false );
					}
				const FileNameAttr& attr = file_info.file_name_list[ name_idx ];
				CHECK_FMT( ( attr.file_attributes & FILE_ATTR_I30_INDEX_PRESENT ) == FILE_ATTR_I30_INDEX_PRESENT );
				if ( attr.name == L"." ) { // root directory
					full_path.insert( 0, add_trailing_slash( volume->name ) );
					attr_list.item( attr_idx ).name = full_path;
					break;
					}
				else {
					full_path.insert( 0, add_trailing_slash( attr.name ) );
					parent_dir_ref = attr.parent_directory;
					}
				}
			}
		fn_idx++;
		}
	}

u64 FileInfo::load_mft_record( u64 mft_rec_num, Array<u8>& ntfs_file_rec_buf ) {
	NTFS_FILE_RECORD_INPUT_BUFFER ntfs_file_rec_in;
	ntfs_file_rec_in.FileReferenceNumber.QuadPart = mft_rec_num;

	unsigned ntfs_file_rec_out_size = NTFS_FILE_REC_HEADER_SIZE + volume->file_rec_size;

	DWORD bytes_ret;
	CHECK_SYS( DeviceIoControl( volume->handle, FSCTL_GET_NTFS_FILE_RECORD, &ntfs_file_rec_in, sizeof( ntfs_file_rec_in ), ntfs_file_rec_buf.buf( ntfs_file_rec_out_size ), ntfs_file_rec_out_size, &bytes_ret, NULL ) );
	ntfs_file_rec_buf.set_size( bytes_ret );

	CHECK_FMT( ntfs_file_rec_buf.size( ) >= NTFS_FILE_REC_HEADER_SIZE );
	const NTFS_FILE_RECORD_OUTPUT_BUFFER* ntfs_file_rec_out = reinterpret_cast< const NTFS_FILE_RECORD_OUTPUT_BUFFER* >( ntfs_file_rec_buf.data( ) );
	CHECK_FMT( ntfs_file_rec_buf.size( ) == NTFS_FILE_REC_HEADER_SIZE + ntfs_file_rec_out->FileRecordLength );
	CHECK_FMT( ntfs_file_rec_out->FileRecordLength >= sizeof( MFT_RECORD ) );

	mft_rec_num = FILE_REF( ntfs_file_rec_out->FileReferenceNumber.QuadPart );

	ntfs_file_rec_buf.remove( 0, NTFS_FILE_REC_HEADER_SIZE );

	const MFT_RECORD* mft_rec = reinterpret_cast< const MFT_RECORD* >( ntfs_file_rec_buf.data( ) );
	CHECK_FMT( mft_rec->magic == magic_FILE );

	return mft_rec_num;
	}

unsigned FileInfo::find_attribute( const Array<u8>& ntfs_file_rec_buf, u32 type, u16 instance ) {
	const MFT_RECORD* mft_rec = reinterpret_cast< const MFT_RECORD* >( ntfs_file_rec_buf.data( ) );

	unsigned attr_off = mft_rec->attrs_offset;
	while ( true ) {
		const ATTR_HEADER* attr_header = reinterpret_cast< const ATTR_HEADER* >( ntfs_file_rec_buf.data( ) + attr_off );
		CHECK_FMT( attr_off + sizeof( attr_header->type ) <= ntfs_file_rec_buf.size( ) );
		if ( attr_header->type == AT_END ) {
			return -1; // not found
			}
		CHECK_FMT( attr_off + sizeof( ATTR_HEADER ) <= ntfs_file_rec_buf.size( ) );

		if ( type == attr_header->type ) {
			if ( ( instance == 0 ) || ( instance == attr_header->instance ) ) {
				return attr_off; // found
				}
			}

		CHECK_FMT( attr_header->length != 0 ); // prevent infinite loop
		attr_off += attr_header->length;
		}
	}

Array<FileInfo::DataRun> FileInfo::decode_data_runs( const Array<u8>& ntfs_file_rec_buf, unsigned attr_off ) {
	Array<DataRun> data_run_list;

	CHECK_FMT( attr_off + sizeof( ATTR_HEADER ) + sizeof( ATTR_NONRESIDENT ) <= ntfs_file_rec_buf.size( ) );
	const ATTR_NONRESIDENT* attr_info = reinterpret_cast< const ATTR_NONRESIDENT* >( ntfs_file_rec_buf.data( ) + attr_off + sizeof( ATTR_HEADER ) );

	const u8* data_runs = ntfs_file_rec_buf.data( );
	unsigned idx = attr_off + attr_info->mapping_pairs_offset;
	data_run_list.extend( ( ntfs_file_rec_buf.size( ) - idx ) / 3 );
	u64 lcn = 0;
	while ( true ) {
		CHECK_FMT( idx < ntfs_file_rec_buf.size( ) );
		if ( data_runs[ idx ] == 0 ) {
			break; // end marker
			}
		unsigned len_l = data_runs[ idx ] & 0x0F;
		unsigned off_l = ( data_runs[ idx ] & 0xF0 ) >> 4;
		idx++;
		CHECK_FMT( idx + len_l + off_l <= ntfs_file_rec_buf.size( ) );
		u64 len = 0;
		unsigned i;
		for ( i = 0; i < len_l; i++ ) {
			len += static_cast< u64 >( data_runs[ idx++ ] ) << ( i * 8 );
			}
		if ( off_l == 0 ) {
			data_run_list += DataRun( -1, len ); // sparse
			continue;
			}
		s64 off = 0;
		for ( i = 0; i + 1 < off_l; i++ ) {
			off |= static_cast< u64 >( data_runs[ idx++ ] ) << ( i * 8 );
			}
		off |= static_cast< s64 >( static_cast< s8 >( data_runs[ idx++ ] ) ) << ( i * 8 );
		lcn = lcn + off;
		data_run_list += DataRun( lcn, len );
		}

	return data_run_list;
	}

void FileInfo::process_attribute( const Array<u8>& ntfs_file_rec_buf, unsigned attr_off ) {
	const MFT_RECORD* mft_rec = reinterpret_cast< const MFT_RECORD* >( ntfs_file_rec_buf.data( ) );
	const ATTR_HEADER* attr_header = reinterpret_cast< const ATTR_HEADER* >( ntfs_file_rec_buf.data( ) + attr_off );
	CHECK_FMT( attr_off + sizeof( ATTR_HEADER ) <= ntfs_file_rec_buf.size( ) );

	AttrInfo attr;
	attr.type = attr_header->type;
	attr.mft_ext_rec = mft_rec->base_mft_record != 0;
	attr.resident = !attr_header->non_resident;
	attr.compressed = ( attr_header->flags & ATTR_IS_COMPRESSED ) == ATTR_IS_COMPRESSED;
	attr.encrypted = ( attr_header->flags & ATTR_IS_ENCRYPTED ) == ATTR_IS_ENCRYPTED;
	attr.sparse = ( attr_header->flags & ATTR_IS_SPARSE ) == ATTR_IS_SPARSE;
	// attribute name
	if ( attr_header->name_length != 0 ) {
		CHECK_FMT( attr_off + attr_header->name_offset + attr_header->name_length * 2 <= ntfs_file_rec_buf.size( ) );
		attr.name.copy( reinterpret_cast< const wchar_t* >( ntfs_file_rec_buf.data( ) + attr_off + attr_header->name_offset ), attr_header->name_length );
		}
	// resident attribute
	if ( attr.resident ) {
		CHECK_FMT( attr_off + sizeof( ATTR_HEADER ) + sizeof( ATTR_RESIDENT ) <= ntfs_file_rec_buf.size( ) );
		const ATTR_RESIDENT* attr_info = reinterpret_cast< const ATTR_RESIDENT* >( ntfs_file_rec_buf.data( ) + attr_off + sizeof( ATTR_HEADER ) );
		attr.data_size = attr_info->value_length;
		attr.disk_size = 0;
		attr.valid_size = attr.data_size;
		attr.fragments = 0;

		// STANDARD_INFORMATION attribute
		if ( attr.type == AT_STANDARD_INFORMATION ) {
			CHECK_FMT( attr_off + attr_info->value_offset + sizeof( STANDARD_INFORMATION_ATTR ) <= ntfs_file_rec_buf.size( ) );
			const STANDARD_INFORMATION_ATTR* attr = reinterpret_cast< const STANDARD_INFORMATION_ATTR* >( ntfs_file_rec_buf.data( ) + attr_off + attr_info->value_offset );
			std_info.creation_time = attr->creation_time;
			std_info.last_data_change_time = attr->last_data_change_time;
			std_info.last_mft_change_time = attr->last_mft_change_time;
			std_info.last_access_time = attr->last_access_time;
			std_info.file_attributes = attr->file_attributes;
			}

		// FILE_NAME attribute
		if ( attr.type == AT_FILE_NAME ) {
			CHECK_FMT( attr_off + attr_info->value_offset + sizeof( FILE_NAME_ATTR ) <= ntfs_file_rec_buf.size( ) );
			const FILE_NAME_ATTR* fn_attr = reinterpret_cast< const FILE_NAME_ATTR* >( ntfs_file_rec_buf.data( ) + attr_off + attr_info->value_offset );
			CHECK_FMT( attr_off + attr_info->value_offset + sizeof( FILE_NAME_ATTR ) + fn_attr->file_name_length <= ntfs_file_rec_buf.size( ) );
			FileNameAttr file_name_attr;
			file_name_attr.parent_directory = FILE_REF( fn_attr->parent_directory );
			file_name_attr.creation_time = fn_attr->creation_time;
			file_name_attr.last_data_change_time = fn_attr->last_data_change_time;
			file_name_attr.last_mft_change_time = fn_attr->last_mft_change_time;
			file_name_attr.last_access_time = fn_attr->last_access_time;
			file_name_attr.file_attributes = fn_attr->file_attributes;
			file_name_attr.file_name_type = fn_attr->file_name_type;
			file_name_attr.name.copy( reinterpret_cast< const wchar_t* >( ntfs_file_rec_buf.data( ) + attr_off + attr_info->value_offset + sizeof( FILE_NAME_ATTR ) ), fn_attr->file_name_length );
			file_name_list += file_name_attr;
			}

		// REPARSE_POINT attribute
		if ( attr.type == AT_REPARSE_POINT ) {
			unsigned value_off = attr_info->value_offset;

			CHECK_FMT( attr_off + value_off + sizeof( REPARSE_POINT ) <= ntfs_file_rec_buf.size( ) );
			const REPARSE_POINT* reparse_point = reinterpret_cast< const REPARSE_POINT* >( ntfs_file_rec_buf.data( ) + attr_off + value_off );

			if ( reparse_point->reparse_tag == IO_REPARSE_TAG_MOUNT_POINT ) {
				CHECK_FMT( reparse_point->reparse_data_length >= sizeof( MOUNT_POINT ) );
				CHECK_FMT( attr_off + value_off + sizeof( REPARSE_POINT ) + sizeof( MOUNT_POINT ) <= ntfs_file_rec_buf.size( ) );
				const MOUNT_POINT* mnt_point = reinterpret_cast< const MOUNT_POINT* >( ntfs_file_rec_buf.data( ) + attr_off + value_off + sizeof( REPARSE_POINT ) );

				value_off += sizeof( REPARSE_POINT ) + sizeof( MOUNT_POINT ) + mnt_point->subst_name_off;
				CHECK_FMT( attr_off + value_off + mnt_point->subst_name_len <= ntfs_file_rec_buf.size( ) );
				attr.name.copy( reinterpret_cast< const wchar_t* >( ntfs_file_rec_buf.data( ) + attr_off + value_off ), mnt_point->subst_name_len / 2 );
				}
			else if ( reparse_point->reparse_tag == IO_REPARSE_TAG_SYMBOLIC_LINK ) {
				CHECK_FMT( reparse_point->reparse_data_length >= sizeof( SYMBOLIC_LINK ) );
				CHECK_FMT( attr_off + value_off + sizeof( REPARSE_POINT ) + sizeof( SYMBOLIC_LINK ) <= ntfs_file_rec_buf.size( ) );
				const SYMBOLIC_LINK* sym_link = reinterpret_cast< const SYMBOLIC_LINK* >( ntfs_file_rec_buf.data( ) + attr_off + value_off + sizeof( REPARSE_POINT ) );

				value_off += sizeof( REPARSE_POINT ) + sizeof( SYMBOLIC_LINK ) + sym_link->subst_name_off;
				CHECK_FMT( attr_off + value_off + sym_link->subst_name_len <= ntfs_file_rec_buf.size( ) );
				attr.name.copy( reinterpret_cast< const wchar_t* >( ntfs_file_rec_buf.data( ) + attr_off + value_off ), sym_link->subst_name_len / 2 );
				}
			}
		attr_list += attr;
		}
	// non-resident attribute
	else {
		bool extent;
		if ( attr.compressed || attr.sparse ) {
			CHECK_FMT( attr_off + sizeof( ATTR_HEADER ) + sizeof( ATTR_COMPRESSED ) <= ntfs_file_rec_buf.size( ) );
			const ATTR_COMPRESSED* attr_info = reinterpret_cast< const ATTR_COMPRESSED* >( ntfs_file_rec_buf.data( ) + attr_off + sizeof( ATTR_HEADER ) );
			attr.data_size = attr_info->data_size;
			attr.disk_size = attr_info->compressed_size;
			attr.valid_size = attr_info->initialized_size;
			extent = attr_info->lowest_vcn != 0;
			}
		else {
			CHECK_FMT( attr_off + sizeof( ATTR_HEADER ) + sizeof( ATTR_NONRESIDENT ) <= ntfs_file_rec_buf.size( ) );
			const ATTR_NONRESIDENT* attr_info = reinterpret_cast< const ATTR_NONRESIDENT* >( ntfs_file_rec_buf.data( ) + attr_off + sizeof( ATTR_HEADER ) );
			attr.data_size = attr_info->data_size;
			attr.disk_size = attr_info->allocated_size;
			attr.valid_size = attr_info->initialized_size;
			extent = attr_info->lowest_vcn != 0;
			}
		// fragments
		Array<DataRun> data_runs = decode_data_runs( ntfs_file_rec_buf, attr_off );
		u64 fragments = 0;
		if ( !extent ) {
			prev_lcn = 0;
			prev_len = 0;
			}
		for ( unsigned i = 0; i < data_runs.size( ); i++ ) {
			if ( data_runs[ i ].lcn == -1 ) {
				continue;
				}
			if ( prev_lcn + prev_len != data_runs[ i ].lcn ) {
				fragments++;
				}
			prev_lcn = data_runs[ i ].lcn;
			prev_len = data_runs[ i ].len;
			}
		if ( extent ) {
			CHECK_FMT( attr_list.size( ) != 0 );
			CHECK_FMT( attr_list.last( ).type == attr.type );
			attr_list.last_item( ).fragments += fragments;
			}
		else {
			attr.fragments = fragments;
			attr_list += attr;
			}
		}
	}

void FileInfo::process_attr_list_entry( const ATTR_LIST_ENTRY* attr_list_entry, Array<u64>& ext_rec_list ) {
	if ( attr_list_entry->mft_reference == base_file_rec_num ) {
		unsigned attr_off = find_attribute( base_file_rec_buf, attr_list_entry->type, attr_list_entry->instance );
		CHECK_FMT( attr_off != -1 );
		process_attribute( base_file_rec_buf, attr_off );
		}
	else {
		if ( ext_rec_list.search( attr_list_entry->mft_reference ) == -1 ) {
			ext_rec_list += attr_list_entry->mft_reference;
			mft_rec_cnt++;
			}

		load_mft_record( attr_list_entry->mft_reference, ext_file_rec_buf );
		unsigned attr_off = find_attribute( ext_file_rec_buf, attr_list_entry->type, attr_list_entry->instance );
		CHECK_FMT( attr_off != -1 );
		process_attribute( ext_file_rec_buf, attr_off );
		}
	}

void FileInfo::process_base_file_rec( ) {
	attr_list.clear( );
	file_name_list.clear( );
	// is attr. list present?
	unsigned attr_list_off = find_attribute( base_file_rec_buf, AT_ATTRIBUTE_LIST );
	// init. mft record counter
	mft_rec_cnt = 1;
	Array<u64> ext_rec_list;
	// no ATTRIBUTE_LIST - one MFT file record
	if ( attr_list_off == -1 ) {
		const MFT_RECORD* mft_rec = reinterpret_cast< const MFT_RECORD* >( base_file_rec_buf.data( ) );

		// walk over list of attributes stored in a base file record
		unsigned attr_off = mft_rec->attrs_offset;
		while ( true ) {
			const ATTR_HEADER* attr_header = reinterpret_cast< const ATTR_HEADER* >( base_file_rec_buf.data( ) + attr_off );
			CHECK_FMT( attr_off + sizeof( attr_header->type ) <= base_file_rec_buf.size( ) );
			if ( attr_header->type == AT_END ) {
				break; // end of attribute list
				}
			CHECK_FMT( attr_off + sizeof( ATTR_HEADER ) <= base_file_rec_buf.size( ) );

			process_attribute( base_file_rec_buf, attr_off );

			CHECK_FMT( attr_header->length != 0 ); // prevent infinite loop
			attr_off += attr_header->length;
			}
		}
	// ATTRIBUTE_LIST present
	else {
		process_attribute( base_file_rec_buf, attr_list_off );
		const ATTR_HEADER* attr_header = reinterpret_cast< const ATTR_HEADER* >( base_file_rec_buf.data( ) + attr_list_off );
		// non-resident ATTRIBUTE_LIST
		if ( attr_header->non_resident ) {
			CHECK_FMT( attr_list_off + sizeof( ATTR_HEADER ) + sizeof( ATTR_NONRESIDENT ) <= base_file_rec_buf.size( ) );
			const ATTR_NONRESIDENT* attr_info = reinterpret_cast< const ATTR_NONRESIDENT* >( base_file_rec_buf.data( ) + attr_list_off + sizeof( ATTR_HEADER ) );
			CHECK_FMT( attr_info->allocated_size <= MAX_ATTR_LIST_SIZE );
			Array<DataRun> data_runs = decode_data_runs( base_file_rec_buf, attr_list_off );
			// calculate disk size using data runs
			u64 attr_disk_size = 0;
			for ( unsigned i = 0; i < data_runs.size( ); i++ ) {
				CHECK_FMT( data_runs[ i ].lcn != -1 ); // compressed or sparse not allowed
				attr_disk_size += data_runs[ i ].len * volume->cluster_size;
				}
			CHECK_FMT( attr_disk_size == attr_info->allocated_size );
			CHECK_FMT( attr_info->data_size <= attr_info->allocated_size );

			// load attribute list data from disk
			u8* attr_data_buf = static_cast< u8* >( VirtualAlloc( NULL, static_cast< SIZE_T >( attr_disk_size ), MEM_COMMIT, PAGE_READWRITE ) );
			CHECK_SYS( attr_data_buf != NULL );
			CLEAN( u8*, attr_data_buf, CHECK_SYS( VirtualFree( attr_data_buf, 0, MEM_RELEASE ) ) );

			unsigned buf_pos = 0;
			for ( unsigned i = 0; i < data_runs.size( ); i++ ) {
				LARGE_INTEGER pos;
				pos.QuadPart = data_runs[ i ].lcn * volume->cluster_size;
				CHECK_SYS( SetFilePointerEx( volume->handle, pos, NULL, FILE_BEGIN ) );

				unsigned size = static_cast< unsigned >( data_runs[ i ].len * volume->cluster_size );
				DWORD bytes_ret;
				volume->flush( );
				CHECK_SYS( ReadFile( volume->handle, attr_data_buf + buf_pos, size, &bytes_ret, NULL ) );
				CHECK_FMT( size == bytes_ret );
				buf_pos += size;
				}

			// process attribute list entries
			unsigned idx = 0;
			while ( idx != attr_info->data_size ) {
				CHECK_FMT( idx + sizeof( ATTR_LIST_ENTRY ) <= attr_info->data_size );
				const ATTR_LIST_ENTRY* attr_list_entry = reinterpret_cast< const ATTR_LIST_ENTRY* >( attr_data_buf + idx );

				process_attr_list_entry( attr_list_entry, ext_rec_list );

				CHECK_FMT( attr_list_entry->length != 0 );
				idx += attr_list_entry->length;
				}
			}
		// resident ATTRIBUTE_LIST
		else {
			CHECK_FMT( attr_list_off + sizeof( ATTR_HEADER ) + sizeof( ATTR_RESIDENT ) <= base_file_rec_buf.size( ) );
			const ATTR_RESIDENT* attr_info = reinterpret_cast< const ATTR_RESIDENT* >( base_file_rec_buf.data( ) + attr_list_off + sizeof( ATTR_HEADER ) );
			unsigned idx = attr_info->value_offset;

			// process attribute list entries
			while ( idx != attr_info->value_offset + attr_info->value_length ) {
				CHECK_FMT( idx < attr_info->value_offset + attr_info->value_length );
				CHECK_FMT( attr_list_off + idx + sizeof( ATTR_LIST_ENTRY ) <= base_file_rec_buf.size( ) );
				const ATTR_LIST_ENTRY* attr_list_entry = reinterpret_cast< const ATTR_LIST_ENTRY* >( base_file_rec_buf.data( ) + attr_list_off + idx );

				process_attr_list_entry( attr_list_entry, ext_rec_list );

				CHECK_FMT( attr_list_entry->length != 0 );
				idx += attr_list_entry->length;
				}
			}
		}
	}