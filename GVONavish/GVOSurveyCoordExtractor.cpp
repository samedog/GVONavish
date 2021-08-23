#include "stdafx.h"
#include "GVONavish.h"
#include "GVOSurveyCoordExtractor.h"

namespace {
	// 縦11x横5ピクセルの二値化画像として画像を解析。
	const int k_numberWidth = 5;
	const std::vector<std::string> k_sampleBits = {
		"00111111100"
		"01000000010"
		"01000000010"
		"00111111100"
		"00000000000",	// 0

		"00100000000"
		"01111111110"
		"00000000000"
		"00000000000"
		"00000000000",	// 1

		"00110000110"
		"01000011010"
		"01000100010"
		"00111000010"
		"00000000000",	// 2

		"00110001100"
		"01000100010"
		"01000100010"
		"00111011100"
		"00000000000",	// 3

		"00000011000"
		"00001101000"
		"00110001000"
		"01111111110"
		"00000001000",	// 4

		"01111101100"
		"01001000010"
		"01001000010"
		"01000111100"
		"00000000000",	// 5

		"00111111100"
		"01000100010"
		"01000100010"
		"00110011100"
		"00000000000",	// 6

		"01000000000"
		"01000001110"
		"01001110000"
		"01110000000"
		"00000000000",	// 7

		"00111011100"
		"01000100010"
		"01000100010"
		"00111011100"
		"00000000000",	// 8

		"00111001100"
		"01000100010"
		"01000100010"
		"00111111100"
		"00000000000",	// 9
	};

};


GVOSurveyCoordExtractor::GVOSurveyCoordExtractor( const GVOImage& image )
: m_image( image )
, m_width( image.size().cx )
, m_height( image.size().cy )
, m_extractOffset()
{
}


GVOSurveyCoordExtractor::~GVOSurveyCoordExtractor()
{
}


std::vector<int> GVOSurveyCoordExtractor::extractNumbers()
{
	std::vector<int> values;

	// 解析処理は縦11pixelsのみ対応
	if ( m_height == 11 ) {
		resetExtractState();

		values = extractNumbersForHeight11();
	}

#ifndef NDEBUG
	// デバッグ用に二値化画像に変換
	{
		const uint8_t * s = &m_binalizedImage[0];
		uint8_t * d = const_cast<GVOImage&>(m_image).mutableImageBits();
		for ( uint32_t i = 0; i < m_binalizedImage.size(); ++i ) {
			const uint8_t v = *s++;
			*d++ = v;
			*d++ = v;
			*d++ = v;
		}
	}
#endif
	return values;
}


std::vector<int> GVOSurveyCoordExtractor::extractNumbersForHeight11()
{
	const int dxThreshold = int( k_numberWidth +4 );
	std::vector<int> values;
	std::string number;

	while ( m_extractOffset < m_width ) {
		const int prevOffset = m_extractOffset;
		const int v = extractOneNumbersForHeight11();
		const int dx = m_extractOffset - prevOffset;

		if ( dxThreshold < dx ) {
			if ( 0 < number.length() ) {
				values.push_back( std::stoi(number) );
			}
			number = "";
		}
		if ( 0 <= v && v <= 9 ) {
			number += std::to_string( v );
		}
	}
	if ( 0 < number.length() ) {
		values.push_back( std::stoi( number ) );
	}
	return values;
}


int GVOSurveyCoordExtractor::extractOneNumbersForHeight11()
{
	const std::vector<uint8_t>& binalizedImage = binalizeImage();
	const size_t maskLength = m_height * k_numberWidth;

	bool found = false;
	std::string bitString;

	BitsDictionary candidates;
	resetCandidates( candidates );

	// 二値化画像を縦に走査
	for ( uint32_t x = m_extractOffset; x < m_width; ++x ) {
		uint32_t vert = 0;
		std::string vertString;

		for ( uint32_t y = 0; y < m_height; ++y ) {
			const uint8_t v = binalizedImage[y * m_width + x] ? 1 : 0;
			vert = (vert << 1) | v;
			vertString += (v) ? '1' : '0';
		}
		// 一色なら解析しない
		if ( !found ) {
			if ( vert == 0 || vert == 0x3FF ) {
				continue;
			}
			found = true;
		}

		bitString += vertString;

		if ( bitString.size() < maskLength ) {
			for ( size_t i = 0; i < k_sampleBits.size(); ++i ) {
				const std::string& sample = k_sampleBits[i];
				if ( sample.compare( 0, bitString.length(), bitString ) == 0 ) {
					candidates[&sample] = i;
				}
				else {
					candidates.erase( &sample );
				}
			}
			if ( candidates.empty() ) {
				bitString = "";
				resetCandidates( candidates );
			}
			continue;
		}
		else {
			for ( BitsDictionary::iterator it = candidates.begin(); it != candidates.end(); ++it ) {
				const std::string &candidate = *it->first;
				const int number = it->second;
				if ( candidate.compare( bitString ) == 0 ) {
					m_extractOffset = x + 1;
					return number;
				}
			}
			// 数字を見つけられなかった
			break;
		}
	}
	m_extractOffset = m_width;
	return -1;
}


void GVOSurveyCoordExtractor::resetExtractState()
{
	m_extractOffset = 0;
}


void GVOSurveyCoordExtractor::resetCandidates( BitsDictionary& bitsDictionary )
{
	bitsDictionary.clear();
	for ( size_t i = 0; i < k_sampleBits.size(); ++i ) {
		bitsDictionary[&k_sampleBits[i]] = i;
	}
}


std::vector<uint8_t> GVOSurveyCoordExtractor::binalizeImage()
{
	if ( m_binalizedImage.empty() ) {
		const uint32_t bytesPerPixel = 3;	// RGB 24bit決め打ち
		const uint32_t stride = m_width * bytesPerPixel;
		const uint8_t * const bits = m_image.imageBits();

		std::vector<uint8_t> binalizedImage;
		binalizedImage.resize( m_width * m_height );
		for ( size_t i = 0; i < binalizedImage.size(); ++i ) {
			const uint32_t offset = i * bytesPerPixel;
			const uint16_t r = bits[offset + 0];
			const uint16_t g = bits[offset + 1];
			const uint16_t b = bits[offset + 2];
			// 適当に計算
			const uint16_t total = r + g + b;
			binalizedImage[i] = ((240 * 3) <= total) ? 255 : 0;
		}
		binalizedImage.swap( m_binalizedImage );
	}
	return m_binalizedImage;
}
