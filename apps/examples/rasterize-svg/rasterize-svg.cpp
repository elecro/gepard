/* Copyright (C) 2018, Gepard Graphics
 * Copyright (C) 2018, Szilard Ledan <szledan@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "arg-parser.h"
#include "gepard.h"
#include "gepard-types.h"
#define NANOSVG_IMPLEMENTATION
#include "nanosvg.h"
#include "surfaces/gepard-png-surface.h"
#include "surfaces/gepard-xsurface.h"
#include <chrono>
#include <iostream>
#include <string>
#include <sstream>
#include <thread>

void parseNSVGimage(gepard::Gepard& ctx, const NSVGimage* img);

int main(int argc, char* argv[])
{
    const bool a_help = PARSE_HELP_AND_USE("-h, --help, --usage", "show this help.", "SVG rastering with Gepard *** (C) 2018. Szilard Ledan\nUsage: %p [options] [SVGFILE=tiger.svg]\n\nOptions:");
    const struct {
        const bool isOn;
        const std::string file;
    } a_png = {
        CHECK_FLAG("-p, --png", argc, argv),
        PARSE_FLAG("-p, --png FILE", std::string("build/tiger.png"), "use png output and set file name; def is '%d'.")
    };
    const std::string a_svgFile = PARSE_ARG(std::string("./apps/examples/rasterize-svg/tiger.svg"));
    if (a_help)
        return 0;

    NSVGimage* pImage = nsvgParseFromFile(a_svgFile.c_str(), "px", 96);
    if (!pImage) {
        std::cerr << "Wrong SVG file or not exist." << std::endl;
        return 1;
    }

    const uint width = pImage->width;
    const uint height = pImage->height;
    gepard::Surface* surface = a_png.isOn ? reinterpret_cast<gepard::Surface*>(new gepard::PNGSurface(width, height))
                                          : reinterpret_cast<gepard::Surface*>(new gepard::XSurface(width, height));
    gepard::Gepard gepard(surface);

    // Clear the canvas.
    gepard.fillStyle = "#fff";
    gepard.fillRect(0, 0, width, height);

    parseNSVGimage(gepard, pImage);

    nsvgDelete(pImage);

    if (a_png.isOn) {
        gepard::PNGSurface* pngSurface = reinterpret_cast<gepard::PNGSurface*>(surface);
        pngSurface->save(a_png.file);
        if (pngSurface) {
            delete pngSurface;
        }
    } else {
        gepard::XSurface* xSurface = reinterpret_cast<gepard::XSurface*>(surface);

        XEvent xEvent;
        while (true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            if (XCheckWindowEvent((Display*)xSurface->getDisplay(), (Window)xSurface->getWindow(), KeyPress | ClientMessage, &xEvent)) {
                break;
            }
        }
        if (xSurface) {
            delete xSurface;
        }
    }

    return 0;
}

const std::string strBounds(const float b[4], const int level = 1)
{
    if (level < 1)
        return "...";

    std::stringstream ss;
    ss << "[" << b[0] << ", " << b[1] << ", " << b[2] << ", " << b[3] << "]";
    return ss.str();
}

const std::string strNSVGpaint(const NSVGpaint& p, const int level = 1)
{
    if (level < 1)
        return "...";

    std::stringstream ss;
    ss << "[type: " << (int)p.type
       << "; color: " << std::hex << p.color
       << "]";
    return ss.str();
}

const std::string strNSVGpath(const NSVGpath* p, const int level = 1)
{
    if (level < 1)
        return "...";

    const int newLevel = level - 1;
    std::stringstream ss;
    ss << "[npts: " << p->npts
       << "; closed: " << (int)p->closed
       << "; bounds: " << strBounds(p->bounds, newLevel)
       << "; next : " << p->next
       << "]";
    return ss.str();
}

const std::string strNSVGshape(const NSVGshape* s, const int level = 1)
{
    if (level < 1)
        return "...";

    const int newLevel = level - 1;
    std::stringstream ss;
    ss << "[id: " << std::string(s->id)
       << "; fill: " << strNSVGpaint(s->fill, newLevel)
       << "; stroke: " << strNSVGpaint(s->stroke, newLevel)
       << "; opacity: " << s->opacity
       << "; strokeWidth: " << s->strokeWidth
       << "; strokeDashOffset: " << s->strokeDashOffset
       << "; strokeDashArray[8]: " << s->strokeDashArray
       << "; strokeDashCount: " << (int)s->strokeDashCount
       << "; strokeLineJoin: " << (int)s->strokeLineJoin
       << "; strokeLineCap: " << (int)s->strokeLineCap
       << "; miterLimit: " << s->miterLimit
       << "; fillRule : " << (int)s->fillRule
       << "; flags: " << s->flags
       << "; bounds: " << strBounds(s->bounds, newLevel)
       << "; path : " << strNSVGpath(s->paths, newLevel)
       << "; next : " << s->next
       << "]";
    return ss.str();
}

const std::string strNSVGimage(const NSVGimage* i, const int level = 1)
{
    if (level < 1)
        return "";

    const int newLevel = level - 1;
    std::stringstream ss;
    ss << "[width: " << i->width
       << "; height: " << i->height
       << "; shapes: ";
    if  (newLevel > 0) {
        NSVGshape* shp = i->shapes;
        while (shp) {
            ss << strNSVGshape(shp, newLevel) << (shp->next ? ", " : "");
            shp = shp->next;
        }
    } else {
        ss << "...";
    }
    ss << "]";
    return ss.str();
}

void parseNSVGimage(gepard::Gepard& ctx, const NSVGimage* img)
{
    NSVGshape* shp = img->shapes;
    while (shp) {
        NSVGpath* pth = shp->paths;
        while(pth) {
            ctx.beginPath();
            float* pts = pth->pts;
            ctx.moveTo(pts[0], pts[1]);
            for (int i = 1; i < pth->npts; i += 3) {
                ctx.bezierCurveTo(pts[i * 2], pts[i * 2 + 1], pts[i * 2 + 2], pts[i * 2 + 3], pts[i * 2 + 4], pts[i * 2 + 5]);
            }
            if (pth->closed) {
                ctx.closePath();
            }
            pth = pth->next;
        }

        if (shp->fill.type != NSVG_PAINT_NONE) {
            gepard::Color fillColor = gepard::Color::fromRawDataABGR(shp->fill.color);
            ctx.setFillColor(fillColor.r * 255 * shp->opacity, fillColor.g * 255 * shp->opacity, fillColor.b * 255 * shp->opacity, fillColor.a * shp->opacity);
            ctx.fill();
        }
        if (shp->stroke.type != NSVG_PAINT_NONE) {
            gepard::Color strokeColor = gepard::Color::fromRawDataABGR(shp->stroke.color);
            ctx.setStrokeColor(strokeColor.r * 255 * shp->opacity, strokeColor.g * 255 * shp->opacity, strokeColor.b * 255 * shp->opacity, strokeColor.a * shp->opacity);
            ctx.lineWidth = shp->strokeWidth / 2.0;
            // strokeDashOffset strokeDashArray strokeDashCount
            switch (shp->strokeLineJoin) {
            case NSVG_JOIN_MITER: ctx.lineJoin = "miter"; break;
            case NSVG_JOIN_ROUND: ctx.lineJoin = "round"; break;
            case NSVG_JOIN_BEVEL: ctx.lineJoin = "bevel"; break;
            }
            switch (shp->strokeLineCap) {
            case NSVG_CAP_BUTT: ctx.lineCap = "butt"; break;
            case NSVG_CAP_ROUND: ctx.lineCap = "round"; break;
            case NSVG_CAP_SQUARE: ctx.lineCap = "square"; break;
            }
            ctx.miterLimit = shp->miterLimit;
            ctx.stroke();
        }
        shp = shp->next;
    }
}

void bug(gepard::Gepard& ctx)
{
    if (1) { // in gepard-trapezoid-tessellator.cpp:737 GD_ASSERT(trapezoid.topY == (fixPrecision(segment.topY() / denom)));
        ctx.beginPath();
        ctx.moveTo(151.33, 481.5);
        ctx.quadraticCurveTo(151.33, 481.5, 151.81, 481.7);
        ctx.quadraticCurveTo(152.29, 481.91, 152.83, 482.25);
        ctx.fill();
        ctx.stroke();
    return;
    }
}
