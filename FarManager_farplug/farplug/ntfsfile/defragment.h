#pragma once

class IDefragProgress {
	public:
	unsigned __int64 total_clusters;
	unsigned __int64 moved_clusters;
	unsigned extents_before;
	unsigned extents_after;
	virtual void update_defrag_ui( bool force = false ) = 0;
	};

void defragment( const std::string& file_name, IDefragProgress& progress );
void defragment( const std::vector<std::string>& file_list, Log& log );
