#pragma once

class GVONoncopyable {
public:
	GVONoncopyable() = default;
	GVONoncopyable( const GVONoncopyable & ) = delete;
	GVONoncopyable& operator=(const GVONoncopyable&) = delete;
	GVONoncopyable( const GVONoncopyable && ) = delete;
	GVONoncopyable& operator=(const GVONoncopyable&&) = delete;
};

