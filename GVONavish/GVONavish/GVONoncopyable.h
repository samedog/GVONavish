#pragma once

class GVONoncopyable {
private:
	GVONoncopyable() = default;
	GVONoncopyable( const GVONoncopyable & ) = delete;
	GVONoncopyable& operator=(const GVONoncopyable&) = delete;
	GVONoncopyable( const GVONoncopyable && ) = delete;
	GVONoncopyable& operator=(const GVONoncopyable&&) = delete;
};

