#pragma once
#include "SearchRequestData.h"

class SearchRequest
{
public:
	SearchRequest(const SearchRequestData& data);
	const SearchRequestData& GetData() const { return m_Data; }

	void ExecuteSearchRequest();
	void StopSearchRequest();

private:
	SearchRequestData m_Data;
};