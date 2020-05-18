#pragma once

#include <QDebug>
#include <QElapsedTimer>

#include <cstdint>
#include <ratio>

static constexpr uint16_t THUMBNAIL_WIDTH = 160;
static constexpr uint16_t THUMBNAIL_HEIGHT = (THUMBNAIL_WIDTH * 3) / 4;
static_assert((THUMBNAIL_HEIGHT * 4) / 3 == THUMBNAIL_WIDTH); // assert whole numbers

static constexpr char IMAGE_SOURCE_PATH [] = "s";
static constexpr char IMAGE_FULL_PATH [] = "i";
static constexpr char IMAGE_THUMB_PATH [] = "t";

static constexpr char SOURCE_SAVE_EXT [] = ".png";
static constexpr char SOURCE_SAVE_FORMAT [] = "PNG";
static constexpr int  SOURCE_QUALITY = 100;

static constexpr char IMAGE_SAVE_EXT [] = ".webp";
static constexpr char IMAGE_SAVE_FORMAT [] = "WEBP";
static constexpr int  IMAGE_QUALITY = 99;

static constexpr char THUMB_SAVE_EXT [] = ".webp";
static constexpr char THUMB_SAVE_FORMAT [] = "WEBP";
static constexpr int  THUMB_QUALITY = 90;

struct PerfProf : public QElapsedTimer {
	inline PerfProf() : QElapsedTimer() { start(); }
	inline void mark(char const * msg) { 
		auto val = nsecsElapsed();
		if (val > std::nano::den) qDebug() << msg << nsecsElapsed() / static_cast<double>(std::nano::den) << "seconds";
		else if (val > std::milli::den) qDebug() << msg << nsecsElapsed() / static_cast<double>(std::micro::den) << "milliseconds";
		else qDebug() << msg << nsecsElapsed() << "nanoseconds";
	}
};
