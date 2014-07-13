#pragma once
#include "item.h"

class CItem;

class CTreeBranch : public CItem {
	public:
		CTreeBranch  ( ITEMTYPE type, LPCTSTR name, bool dontFollow = false );
		~CTreeBranch (                                                      );
		CTreeBranch  ( CTreeBranch&&  in                                    );

		//Functions that work with branch items
		LONGLONG GetProgressRangeMyComputer    (                                          ) const;//const return type?
		LONGLONG GetProgressPosMyComputer      (                                          ) const;
		INT_PTR  FindFreeSpaceItemIndex        (                                          ) const;
		INT_PTR  FindUnknownItemIndex          (                                          ) const;
		void AddDirectory                      ( _In_ const CFileFindWDS& finder          );
		void AddFile                           ( _In_ const FILEINFO&     fi              );
		LONGLONG GetProgressPosDrive           (                                          ) const;

		//Functions that work with branch items
		_Success_(return != NULL) _Must_inspect_result_  CTreeBranch* FindDirectoryByPath              ( _In_ const CString& path                         );
		_Success_(return != NULL) _Must_inspect_result_  CItem* FindFreeSpaceItem                (                                                  ) const;
		_Success_(return != NULL) _Must_inspect_result_  CItem* FindUnknownItem                  (                                                  ) const;
		_Success_(return != NULL) _Must_inspect_result_  CItem* GetChild                         ( _In_ const INT i                                 ) const;
		_Success_(return != NULL)                        CItem* GetChildGuaranteedValid          ( _In_ const INT_PTR i                             ) const;
		bool     MustShowReadJobs              (                                          ) const;
		INT_PTR FindChildIndex             ( _In_ const CItem* child                                       ) const;
		void AddChild                      ( _In_       CItem*             child                           );
		void AddChild                      ( _In_       CTreeBranch*             child                           );
#ifdef CHILDVEC
		void AddChildToVec                 ( _In_       CItem&             child                           );
#endif
		void AddTicksWorked                ( _In_ const std::uint64_t more                            );
		void CreateFreeSpaceItem           (                                                               );
		void CreateUnknownItem             (                                                               );
		void DoSomeWork                    ( _In_ const unsigned long long ticks                           );
		void readJobNotDoneWork            ( _In_ const unsigned long long ticks, _In_ unsigned long long start );
		void FindFilesLoop                 ( _In_ const unsigned long long ticks, _In_ unsigned long long start, _Inout_ LONGLONG& dirCount, _Inout_ LONGLONG& fileCount, _Inout_ std::vector<FILEINFO>& files );
		void RemoveAllChildren             (                                                               );
		void RemoveAllChildrenFromVec      (                                                               );
		void RemoveChild                   ( _In_ const INT_PTR                i                           );
		void RemoveChildFromVec            ( _In_ const size_t             i                               );
		void RemoveFreeSpaceItem           (                                                               );
		void RemoveUnknownItem             (                                                               );
		void SetDone                       (                                                               );
		void SetReadJobDone                ( _In_ const bool               done = true                     );
		void StillHaveTimeToWork           ( _In_ const unsigned long long ticks, _In_ unsigned long long start );
		void stdRecurseCollectExtensionData( _Inout_ std::vector<SExtensionRecord>& extensionRecords       );
		std::uint64_t GetTicksWorked       ( ) const;
		void StartRefreshUpwardClearItem   ( _In_ ITEMTYPE typeOf_thisItem );
		void UpdateFreeSpaceItem           (                                                               );
		//void UpdateLastChange              (                                                               );
		void UpwardAddSubdirs              ( _In_ const std::int64_t      dirCount                        );
		void UpwardAddFiles                ( _In_ const std::int64_t      fileCount                       );
		void UpwardAddSize                 ( _In_ const std::int64_t      bytes                           );
		void UpwardAddReadJobs             ( _In_ const std::int64_t      count                           );
		void UpwardUpdateLastChange        ( _In_ const FILETIME&          t                               );
		void UpwardRecalcLastChange        (                                                               );
		void UpwardSetUndone               (                                                               );
		void UpwardSetUndoneIT_DRIVE       (                                                               );
		void UpwardParentSetUndone         (                                                               );
		LONGLONG GetProgressRange        (                                  ) const;
		LONGLONG GetProgressPos          (                                  ) const;
		LONGLONG GetReadJobs             (                                  ) const;
		LONGLONG GetFilesCount           (                                  ) const;
		LONGLONG GetSubdirsCount         (                                  ) const;
		LONGLONG GetItemsCount           (                                  ) const;
 inline bool IsDone                      (                                  ) const;
 inline bool IsRootItem                  (                                  ) const;
 inline bool IsReadJobDone               (                                  ) const;
		bool StartRefresh                (                                  );
		bool StartRefreshIT_MYCOMPUTER   ( );
		bool StartRefreshIT_FILESFOLDER  ( _In_ bool wasExpanded );
		void StartRefreshHandleDeletedItem ( );
		void StartRefreshRecreateFSandUnknw( );
		_Must_inspect_result_                     bool          StartRefreshIsMountOrJunction    ( _In_ ITEMTYPE typeOf_thisItem                    );

		// CTreeListItem Interface
		virtual void                                   DrawAdditionalState       ( _In_       CDC*           pdc,        _In_ const CRect& rcLabel                                                                                         ) const;

		virtual bool                                   DrawSubitem               ( _In_ const INT            subitem,    _In_       CDC*   pdc,    _Inout_ CRect& rc, _In_ const UINT state, _Inout_opt_ INT* width, _Inout_ INT* focusLeft) const;		
		virtual INT                                    CompareSibling            ( _In_ const CTreeListItem* tlib,       _In_ const INT    subitem                                                                                         ) const;
		virtual INT_PTR                                GetChildrenCount          (                                                                                                                                                         ) const;
		_Must_inspect_result_ virtual CTreeListItem*   GetTreeListChild          ( _In_ const INT            i                                                                                                                             ) const;
		virtual CString                                GetText                   ( _In_ const INT            subitem                                                                                                                       ) const;

		//virtual INT                                    GetImageToCache           (                                                                                                                                                         ) const;
		//virtual COLORREF                               GetItemTextColor          (                                                                                                                                                         ) const;
		INT CompareSubTreePercentage( _In_ const CTreeBranch* other ) const;

		void GetTextCOL_SUBTREEPERCENTAGE( _Inout_ CString& s ) const;
		void GetTextCOL_PERCENTAGE       ( _Inout_ CString& s ) const;//COL_ITEMS
		void GetTextCOL_ITEMS            ( _Inout_ CString& s ) const;
		void GetTextCOL_FILES            ( _Inout_ CString& s ) const;
		void GetTextCOL_SUBDIRS          ( _Inout_ CString& s ) const;
		//void GetTextCOL_LASTCHANGE       ( _Inout_ CString& s ) const;
		//void GetTextCOL_ATTRIBUTES       ( _Inout_ CString& s ) const;

		
		

		// CTreemap::Item interface
		virtual CRect            TmiGetRectangle                                 (                                                                                                  ) const;
		virtual void             TmiSetRectangle                                 ( _In_ const CRect&         rc                                                                             );

		// CTreemap::Item interface -> header-implemented functions
		
		virtual bool                                   TmiIsLeaf                 (                             )   const { return IsLeaf          ( GetType( ) ); }
		virtual COLORREF                               TmiGetGraphColor          (                             )   const { return GetGraphColor   (            ); }
		virtual LONGLONG                               TmiGetSize                (                             )   const { return GetSize         (            ); }

		_Must_inspect_result_ virtual CTreemap::Item  *TmiGetChild               (      const INT            c ) { return dynamic_cast<CTreemap::Item*>(GetChild        ( c          )); }
		virtual INT_PTR                                TmiGetChildrenCount       (                             )   const { return GetChildrenCount(            ); }

		// CItem
		static  INT                                    GetSubtreePercentageWidth (                             );


	protected:
#ifdef CHILDVEC
		//std::vector<CItem>       m_vectorOfChildren;
		std::vector<CItem*>       m_children;

#else
		CArray<CItem *, CItem *> m_children;
#endif

		//4,294,967,295  (4294967295 ) is the maximum number of files in an NTFS filesystem according to http://technet.microsoft.com/en-us/library/cc781134(v=ws.10).aspx
		//18446744073709551615 is the maximum theoretical size of an NTFS file              according to http://blogs.msdn.com/b/oldnewthing/archive/2007/12/04/6648243.aspx

												 ITEMTYPE           m_type;				// Indicates our type. See ITEMTYPE.
												 unsigned char      m_attributes;		// Packed file attributes of the item
		                                         bool               m_readJobDone : 1;	// FindFiles() (our own read job) is finished.
		                                         bool				m_done : 1;			// Whole Subtree is done.
		_Field_range_( 0, 4294967295 )		     std::uint32_t      m_files;			// # Files in subtree
												 CString            m_name;				// Display name
		_Field_range_( 0, 4294967295 )		     std::uint32_t      m_subdirs;			// # Folder in subtree
		_Field_range_( 0, 4294967295 )           std::uint32_t		m_readJobs;			// # "read jobs" in subtree.
		_Field_range_( 0, 18446744073709551615 ) LONGLONG			m_size;				// OwnSize, if IT_FILE or IT_FREESPACE, or IT_UNKNOWN; SubtreeTotal else.
											     std::uint32_t		m_ticksWorked;		// ms time spent on this item.





		static_assert( sizeof( LONGLONG ) == sizeof( std::int64_t ),            "y'all ought to check m_size, m_files, m_subdirs, m_freeDiskSpace, m_totalDiskSpace!!" );
		static_assert( sizeof( unsigned long long ) == sizeof( std::uint64_t ), "y'all ought to check m_ticksWorked" );
		static_assert( sizeof( unsigned char ) == 1, "y'all ought to check m_attributes" );
		static_assert( sizeof( short ) == sizeof( std::int16_t ), "check m_rect!" );
		static_assert( sizeof( LONGLONG ) == sizeof( std::int64_t ), "y'all ought to check FILEINFO" );
		static_assert( sizeof( unsigned long long ) == sizeof( std::uint64_t ), "GetTicksWorked has bad return type!" );

	};

