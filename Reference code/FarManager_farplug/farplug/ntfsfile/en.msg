.Language=English,English

plugin.name = <(NAME)>

# File metadata
metadata.col.type = Type
metadata.col.flags = RXCES
metadata.col.data_size = Data Size
metadata.col.disk_size = Disk Size
metadata.col.fragments = Fragments
metadata.col.name_data = Name/Data
metadata.row.unnamed_total = Unnamed DATA Total
metadata.row.named_total = Named DATA Total
metadata.row.nr_total = Non-Resident Total
metadata.file_name = &File:
metadata.multiple = <multiple>
metadata.mft_recs = MFT recs: %u
metadata.hard_links = Hard links: %u
metadata.totals.files = Files: %u
metadata.totals.hl = (+%u hl)
metadata.totals.rp_files = (+%u rp)
metadata.totals.dirs = Directories: %u
metadata.totals.rp_dirs = (+%u rp)
metadata.totals.errors = Errors: %S
metadata.totals.excess_fragments = +%S

# Menu
menu.metadata = &Metadata information
menu.content = &Content analysis
menu.panel.on = Open &panel
menu.panel.off = Close &panel
menu.defragment = &Defragment
menu.file_version = File &version
menu.compress_files = Comp&ress files
menu.flat_mode.on = Flat m&ode
menu.flat_mode.off = Normal m&ode
menu.mft_mode.on = MFT &index on
menu.mft_mode.off = MFT &index off
menu.show_totals = Show &totals

# File panel
file_panel.read_dir.progress.title = Reading directory...
file_panel.read_dir.progress.message = %u objects found
file_panel.read_volume.progress.title = Reading volume...
file_panel.read_volume.progress.message = %u file records found
file_panel.read_volume.progress.console.title = {%u%%} Reading volume...
file_panel.title_prefix = nfi
file_panel.mode.title = Panel mode
file_panel.mode.col_types = Column &types
file_panel.mode.status_col_types = St&atus line column types
file_panel.mode.col_widths = Column &widths
file_panel.mode.status_col_widths = Status l&ine column widths
file_panel.mode.wide = &Fullscreen view
file_panel.show_streams = S&how streams
file_panel.show_main_stream = Show &unnamed DATA streams
file_panel.use_highlighting = Enable files high&lighting
file_panel.use_usn_journal = Use USN &journal
file_panel.use_existing_usn_journal = only when it already &exists
file_panel.delete_usn_journal = &Delete USN journal after use
file_panel.delete_own_usn_journal = only when created by &plugin
file_panel.use_cache = Use MFT index &cache
file_panel.default_mft_mode = Use &MFT index mode by default
file_panel.backward_mft_scan = &Backward MFT scan
file_panel.cache_dir = Cache di&rectory:
file_panel.flat_mode_auto_off = Aut&omatically switch off when changing directory
file_panel.flat_mode_params = Flat mode parameters:
file_panel.use_std_sort = Use standard 'By &Name' sorting
file_panel.mode.sort = &Sort by:
file_panel.mode.sort.nothing = Nothing
file_panel.mode.sort.data_size = Data Size
file_panel.mode.sort.disk_size = Disk Size
file_panel.mode.sort.fragments = Fragments
file_panel.mode.sort.streams = Streams
file_panel.mode.sort.hard_links = Hard Links
file_panel.mode.sort.mft_records = MFT Records
file_panel.mode.sort.fragm_level = Fragm. Level
file_panel.mode.sort.valid_size = Valid Size
file_panel.mode.sort.name_len = Path Length
file_panel.mode.col_title.data_size = Data Sz
file_panel.mode.col_title.disk_size = Disk Sz
file_panel.mode.col_title.fragments = Fragm
file_panel.mode.col_title.streams = Stm
file_panel.mode.col_title.hard_links = Lnk
file_panel.mode.col_title.mft_records = MFT
file_panel.mode.col_title.valid_size = ValidSz
file_panel.error_marker = ERROR
file_panel.write_cache.progress.title = Storing MFT index cache...
file_panel.write_cache.progress.console.title = {%u%%} Storing MFT index cache...
file_panel.read_cache.progress.title = Loading MFT index cache...
file_panel.read_cache.progress.console.title = {%u%%} Loading MFT index cache...
file_panel.update_cache.progress.title = Reading USN journal...
file_panel.update_cache.progress.console.title = {%u%%} Reading USN journal...

# File content analysis settings
content.settings.title = File content analysis
content.settings.compression = &Compression ratio
content.settings.crc32 = CRC&32
content.settings.md5 = MD&5
content.settings.sha1 = SHA&1
content.settings.sha256 = SHA25&6
content.settings.ed2k = ED&2K
content.settings.crc16 = C&RC16
content.settings.set_all = &Set all
content.settings.reset_all = &Reset all

# File content analysis results
content.result.title = File content analysis
content.result.processed1 = Processed %S in %S at %S
content.result.processed2 = Processed %S in %S
content.result.compression = &Compression ratio: %S / %S = 
content.result.crc32 = CRC&32:
content.result.md5 = MD&5:
content.result.sha1 = SHA&1:
content.result.sha256 = SHA25&6:
content.result.ed2k = ED&2K:
content.result.crc16 = C&RC16:
content.result.verify = &Verify:
content.result.result = Result:
content.result.close = Close
content.result.save_file = &Save to file
content.result.correct = Correct
content.result.wrong = Wrong
content.result.file_exists = File already exists. Overwrite?
content.result.file_saved = File saved successfully.

# File content analysis progress
content.progress.title = Processing
content.progress.processed1 = Processed %8S from %8S [%3u%%] at %10S
content.progress.processed2 = Processed %8S from %8S [%3u%%]
content.progress.elapsed = Elapsed: %8S  Left: %8S  Total: %8S
content.progress.compression = Compression ratio: %8S / %8S = %3u%%
content.progress.console.title = {%u%%} Processing...

# Multiple file content analysis result
content.multi.result.title = File content analysis
content.multi.result.processed = Processed %S in %S
content.multi.result.speed = at %S
content.multi.result.compression = Compression ratio: %S / %S = 
content.multi.result.files = Files processed: %u
content.multi.result.dirs = Directories processed: %u
content.multi.result.reparse = Reparse points skipped: %u
content.multi.result.errors = Errors (objects skipped): %u
content.multi.result.close = Close

# Multiple file content analysis progress
content.multi.progress.title = Processing
content.multi.progress.processed = Processed %S from %S [%u%%]
content.multi.progress.speed = at %S
content.multi.progress.compression = Compression ratio: %S / %S = %Lu%%
content.multi.progress.elapsed = Elapsed: %S  Left: %S  Total: %S
content.multi.progress.files = Files processed: %u from %u
content.multi.progress.dirs = Directories processed: %u from %u
content.multi.progress.reparse = Reparse points skipped: %u
content.multi.progress.errors = Errors (objects skipped): %u
content.multi.progress.console.title = {%u%%} Processing...

# Estimate total size progress
estimate.progress.title = Scanning
estimate.progress.size = Total size: %S
estimate.progress.files = Files: %u
estimate.progress.dirs = Directories: %u
estimate.progress.reparse = Reparse points: %u
estimate.progress.errors = Errors: %u

# Defragmentator
defrag.progress.title = Defragmenting
defrag.progress.file_name = File:
defrag.progress.clusters = Moved %Lu clusters from %Lu total
defrag.progress.extents = Reducing number of extents from %u to %u
defrag.progress.analyze = Analyzing volume
defrag.progress.files = Processed %u files from %u total
defrag.progress.console.title = {%Lu%%} Defragmenting...
defrag.errors = Some files were not defragmented because of errors. See log for details.

# Show totals
show_totals.title = File totals
show_totals.data_size = Size of data streams: %S (%Lu)
show_totals.disk_size = Size on disk: %S (%Lu)
show_totals.fragments = Excess fragments: %Lu (%Lu per file)
show_totals.files = Files: %u (%u have hard links; %u symbolic links)
show_totals.dirs = Directories: %u (%u symbolic links)

# Error log
log.title = Error Log
log.show = &Show error log

# File version
file_ver.title = File version info
file_ver.fixed_file_version = &File version
file_ver.fixed_product_version = &Product version
file_ver.file_flags = File fl&ags
file_ver.file_os = File &OS
file_ver.file_type = File &type
file_ver.file_sub_type = File &subtype
file_ver.machine = Machine
file_ver.os_version = OS version
file_ver.subsystem = Subsystem
file_ver.image_chars = Characteristics
file_ver.dll_chars = DLL characteristics
file_ver.link_time = Link time
file_ver.language = &Language
file_ver.comments = &Comments
file_ver.company_name = Co&mpany name
file_ver.file_description = File &description
file_ver.file_version = File &version
file_ver.internal_name = &Internal name
file_ver.legal_copyright = L&egal copyright
file_ver.legal_trademarks = Le&gal trademarks
file_ver.original_file_name = O&riginal filename
file_ver.product_name = Product &name
file_ver.product_version = Product version
file_ver.private_build = Private b&uild
file_ver.special_build = Special &build
file_ver.signature = Signature
file_ver.program_name = Program name
file_ver.publisher_info = Publisher info
file_ver.more_info = More info
file_ver.signer.issuer = Certificate issuer
file_ver.signer.subject = Certificate subject
file_ver.timestamp.issuer = Timestamp issuer
file_ver.timestamp.subject = Timestamp subject
file_ver.timestamp = Timestamp
file_ver.verify_signature = Verif&y
file_ver.verify_ok = Valid signature
file_ver.verify_bad = Invalid signature
file_ver.verify_in_progress = Wait...
file_ver.props = Summary Information
file_ver.prop.title = Title
file_ver.prop.subject = Subject
file_ver.prop.author = Author
file_ver.prop.comments = Comments
file_ver.no_info = No version information found.

# Compress files
compress_files.title = Compress files
compress_files.min_file_size = Min. file size (MB):
compress_files.max_compression_ratio = Max. compression ratio (%):
compress_files.min_file_age = Min. number of days since last modification:
compress_files.defragment_after_compression = Defragment after compression
compress_files.errors = Some files were not processed because of errors. See log for details.
compress_files.progress.title = Processing
compress_files.progress.console_title = {%u%%} Processing...
compress_files.progress.file_name = File:
compress_files.progress.estimation = Estimating compression ratio: %7S / %7S = %3u%%
compress_files.progress.estimation.size = Processed %7S from %7S [%3u%%]
compress_files.progress.estimation.speed = at %9S
compress_files.progress.compression = Compressing file...
compress_files.progress.compression.ratio = Projected compression ratio: %u%% - %u%%
compress_files.progress.defragment = Reducing number of extents from %u to %u
compress_files.progress.defragment.clusters = Moved %Lu clusters from %Lu total
compress_files.progress.defragment.analyze = Analyzing volume...
compress_files.progress.files = Files processed: %u from %u
compress_files.progress.errors = Errors (objects skipped): %u

# Generic buttons
button.ok = OK
button.cancel = Cancel

# Size and speed suffixes
suffix.size.b = B
suffix.size.kb = KB
suffix.size.mb = MB
suffix.size.gb = GB
suffix.size.tb = TB
suffix.speed.b = B/s
suffix.speed.kb = KB/s
suffix.speed.mb = MB/s
suffix.speed.gb = GB/s
suffix.speed.tb = TB/s
suffix.alt.size.kb = KiB
suffix.alt.size.mb = MiB
suffix.alt.size.gb = GiB
suffix.alt.size.tb = TiB
suffix.alt.speed.kb = KiB/s
suffix.alt.speed.mb = MiB/s
suffix.alt.speed.gb = GiB/s
suffix.alt.speed.tb = TiB/s
suffix.short.size.kb = K
suffix.short.size.mb = M
suffix.short.size.gb = G
suffix.short.size.tb = T
