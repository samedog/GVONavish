#pragma once

#include <cinttypes>
#include <vector>
#include <map>

#include "GVONoncopyable.h"
#include "GVOImage.h"

//!@brief 数値を抽出するクラス
class GVOSurveyCoordExtractor : private GVONoncopyable{
private:
	typedef std::map<const std::string *, int> BitsDictionary;

private:
	const GVOImage& m_image;
	const uint32_t m_width;
	const uint32_t m_height;
	uint32_t m_extractOffset;

	std::vector<uint8_t> m_binalizedImage;

public:

	GVOSurveyCoordExtractor( const GVOImage& image );
	virtual ~GVOSurveyCoordExtractor();

	std::vector<int> extractNumbers();

private:
	std::vector<int> extractNumbersForHeight11();
	int extractOneNumbersForHeight11();
	void resetExtractState();
	void resetCandidates( BitsDictionary& bitsDictionary );
	std::vector<uint8_t> binalizeImage();
};

