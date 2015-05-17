
/*******************************************************************************
*  The "New BSD License" : http://www.opensource.org/licenses/bsd-license.php  *
********************************************************************************

Copyright (c) 2010, Mark Turney
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
	* Redistributions of source code must retain the above copyright
	  notice, this list of conditions and the following disclaimer.
	* Redistributions in binary form must reproduce the above copyright
	  notice, this list of conditions and the following disclaimer in the
	  documentation and/or other materials provided with the distribution.
	* Neither the name of the <organization> nor the
	  names of its contributors may be used to endorse or promote products
	  derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

******************************************************************************/

#pragma once

#include <vector>
#include <string>
#include <sstream>
#include <fstream>

#include <iostream>
#include <array>
#include <initializer_list>
#include <boost/optional.hpp>
#include "concatenate.h"

using boost::optional;
using boost::make_optional;
using std::string;
using std::ostringstream;
using std::vector;

namespace svg {

// Utility XML/String Functions.
template <typename T>
string attribute(const string& attribute_name,
	const T& value,
	const string& unit = "")
{
	ostringstream ss;
	ss << attribute_name << "=\"" << value << unit << "\" ";
	return ss.str();
}
string elemStart(const string& element_name)
{
	return "\t<" + element_name + " ";
}
string elemEnd(const string& element_name)
{
	return "</" + element_name + ">\n";
}
string emptyElemEnd()
{
	return "/>\n";
}

struct Dimensions
{
	Dimensions(double width, double height) : width(width), height(height) { }
	Dimensions(double combined = 0) : width(combined), height(combined) { }
	double width;
	double height;
};

struct Point
{
	Point(double x = 0, double y = 0) : x(x), y(y) { }
	double x;
	double y;
};

optional<Point> getMinPoint(const vector<Point>& points)
{
	if (points.empty())
		return optional<Point>();
	Point min = points[0];
	for (unsigned i = 1; i < points.size(); ++i) {
		auto& pt = points[i];
		if (pt.x < min.x) min.x = pt.x;
		if (pt.y < min.y) min.y = pt.y;
	}
	return make_optional(min);
}

optional<Point> getMaxPoint(const vector<Point>& points)
{
	if (points.empty())
		return optional<Point>();
	Point max = points[0];
	for (unsigned i = 1; i < points.size(); ++i) {
		auto& pt = points[i];
		if (pt.x > max.x) max.x = pt.x;
		if (pt.y > max.y) max.y = pt.y;
	}
	return make_optional(max);
}

// Defines the dimensions, scale, origin, and origin offset of the document.
struct Layout
{
	enum Origin { TopLeft, BottomLeft, TopRight, BottomRight };

	Layout(const Dimensions& dimensions = Dimensions(400, 300),
		Origin origin = BottomLeft,
		double scale = 1,
		const Point& origin_offset = Point(0, 0))
		:
		dimensions(dimensions),
		scale(scale),
		origin(origin),
		origin_offset(origin_offset)
	{ }
	Dimensions dimensions;
	double scale;
	Origin origin;
	Point origin_offset;
};

// Convert coordinates in user space to SVG native space.
double translateX(double x, const Layout& layout)
{
	if (layout.origin == Layout::BottomRight || layout.origin == Layout::TopRight)
		return layout.dimensions.width - ((x + layout.origin_offset.x) * layout.scale);
	else
		return (layout.origin_offset.x + x) * layout.scale;
}

double translateY(double y, const Layout& layout)
{
	if (layout.origin == Layout::BottomLeft || layout.origin == Layout::BottomRight)
		return layout.dimensions.height - ((y + layout.origin_offset.y) * layout.scale);
	else
		return (layout.origin_offset.y + y) * layout.scale;
}

double translateScale(double dimension, const Layout& layout)
{
	return dimension * layout.scale;
}

class Serializeable
{
public:
	Serializeable() { }
	virtual ~Serializeable() { };
	virtual string toString(const Layout& layout) const = 0;
};

class Color : public Serializeable
{
public:
	enum Defaults { Transparent = -1, Aqua, Black, Blue, Brown, Cyan, Fuchsia,
		Green, Lime, Magenta, Orange, Purple, Red, Silver, White, Yellow };

	Color(int r, int g, int b) : transparent(false), red(r), green(g), blue(b) { }
	Color(Defaults color)
		:
		transparent(false),
		red(0),
		green(0),
		blue(0)
	{
		switch (color) {
		case Aqua: assign(0, 255, 255); break;
		case Black: assign(0, 0, 0); break;
		case Blue: assign(0, 0, 255); break;
		case Brown: assign(165, 42, 42); break;
		case Cyan: assign(0, 255, 255); break;
		case Fuchsia: assign(255, 0, 255); break;
		case Green: assign(0, 128, 0); break;
		case Lime: assign(0, 255, 0); break;
		case Magenta: assign(255, 0, 255); break;
		case Orange: assign(255, 165, 0); break;
		case Purple: assign(128, 0, 128); break;
		case Red: assign(255, 0, 0); break;
		case Silver: assign(192, 192, 192); break;
		case White: assign(255, 255, 255); break;
		case Yellow: assign(255, 255, 0); break;
		default: transparent = true; break;
		}
	}
	virtual ~Color() { }
	string toString(const Layout&) const
	{
		if (transparent)
			return "transparent";
		else {
			ostringstream ss;
			ss	<< "rgb("
				<< red << ","
				<< green << ","
				<< blue << ")";
			return ss.str();
		}
	}

private:
	bool transparent;
	int red;
	int green;
	int blue;

	void assign(int r, int g, int b)
	{
		red = r;
		green = g;
		blue = b;
	}
};

class Fill : public Serializeable
{
public:
	Fill(Color::Defaults color) : color(color) { }
	Fill(Color color = Color::Transparent) : color(color) { }
	string toString(const Layout& layout) const
	{
		ostringstream ss;
		ss << attribute("fill", color.toString(layout));
		return ss.str();
	}
private:
	Color color;
};

class Stroke : public Serializeable
{
public:
	Stroke(double width = -1,
		Color color = Color::Transparent)
		:
		width(width),
		color(color)
	{ }
	string toString(const Layout& layout) const
	{
		// If stroke width is invalid.
		if (width < 0)
			return string();
		return concatenate(
			attribute("stroke-width", translateScale(width, layout)),
			attribute("stroke", color.toString(layout))
		);
	}
private:
	double width;
	Color color;
};

class Font : public Serializeable
{
public:
	Font(double size = 12, const string& family = "Verdana") : size(size), family(family) { }
	string toString(const Layout& layout) const
	{
		return concatenate(
			attribute("font-size", translateScale(size, layout)),
			attribute("font-family", family)
			);
	}
private:
	double size;
	string family;
};

class Shape : public Serializeable
{
public:
	Shape(const Fill& fill = Fill(), const Stroke& stroke = Stroke())
		:
		fill(fill),
		stroke(stroke)
	{ }
	virtual ~Shape() { }
	virtual string toString(const Layout& layout) const = 0;
	virtual void offset(const Point& offset) = 0;
protected:
	Fill fill;
	Stroke stroke;
};

template <typename T>
string vectorToString(const vector<T>& collection, const Layout& layout)
{
	string combination_str;
	for (auto& val: collection) {
		combination_str += val.toString(layout);
	}
	return combination_str;
}

class Circle : public Shape
{
public:
	Circle(const Point& center,
		double diameter,
		const Fill& fill,
		const Stroke& stroke = Stroke())
		:
		Shape(fill, stroke),
		center(center),
		radius(diameter / 2)
	{ }
	string toString(const Layout& layout) const
	{
		return concatenate(
			elemStart("circle"),
			attribute("cx", translateX(center.x, layout)),
			attribute("cy", translateY(center.y, layout)),
			attribute("r", translateScale(radius, layout)),
			fill.toString(layout),
			stroke.toString(layout),
			emptyElemEnd()
			);
	}
	void offset(const Point& offset)
	{
		center.x += offset.x;
		center.y += offset.y;
	}
private:
	Point center;
	double radius;
};

class Elipse : public Shape
{
public:
	Elipse(const Point& center,
		double width,
		double height,
		const Fill& fill = Fill(),
		const Stroke& stroke = Stroke())
		:
		Shape(fill, stroke),
		center(center),
		radius_width(width / 2),
		radius_height(height / 2)
	{ }
	string toString(const Layout& layout) const
	{
		return concatenate(
			elemStart("ellipse"),
			attribute("cx", translateX(center.x, layout)),
			attribute("cy", translateY(center.y, layout)),
			attribute("rx", translateScale(radius_width, layout)),
			attribute("ry", translateScale(radius_height, layout)),
			fill.toString(layout),
			stroke.toString(layout),
			emptyElemEnd()
			);
	}
	void offset(const Point& offset)
	{
		center.x += offset.x;
		center.y += offset.y;
	}
private:
	Point center;
	double radius_width;
	double radius_height;
};

class Rectangle : public Shape
{
public:
	Rectangle(const Point& edge,
		double width,
		double height,
		const Fill& fill = Fill(),
		const Stroke& stroke = Stroke())
		:
		Shape(fill, stroke),
		edge(edge),
		width(width),
		height(height)
	{ }
	string toString(const Layout& layout) const
	{
		return concatenate(
			elemStart("rect"),
			attribute("x", translateX(edge.x, layout)),
			attribute("y", translateY(edge.y, layout)),
			attribute("width", translateScale(width, layout)),
			attribute("height", translateScale(height, layout)),
			fill.toString(layout),
			stroke.toString(layout),
			emptyElemEnd()
			);
	}
	void offset(const Point& offset)
	{
		edge.x += offset.x;
		edge.y += offset.y;
	}
private:
	Point edge;
	double width;
	double height;
};

class Line : public Shape
{
public:
	Line(const Point& start_point,
		const Point& end_point,
		const Stroke& stroke = Stroke())
		:
		Shape(Fill(), stroke),
		start_point(start_point),
		end_point(end_point)
	{ }
	string toString(const Layout& layout) const
	{
		return concatenate(
			elemStart("line"),
			attribute("x1", translateX(start_point.x, layout)),
			attribute("y1", translateY(start_point.y, layout)),
			attribute("x2", translateX(end_point.x, layout)),
			attribute("y2", translateY(end_point.y, layout)),
			stroke.toString(layout),
			emptyElemEnd()
			);
	}
	void offset(const Point& offset)
	{
		start_point.x += offset.x;
		start_point.y += offset.y;

		end_point.x += offset.x;
		end_point.y += offset.y;
	}
private:
	Point start_point;
	Point end_point;
};

class Polygon : public Shape
{
public:
	Polygon(const Fill& fill = Fill(), const Stroke& stroke = Stroke())
		:
		Shape(fill, stroke)
	{ }
	Polygon(const Stroke& stroke = Stroke()) : Shape(Color::Transparent, stroke) { }
	Polygon& operator << (const Point& point)
	{
		points.push_back(point);
		return *this;
	}
	string toString(const Layout& layout) const
	{
		ostringstream ss;
		ss	<< elemStart("polygon")
			<< "points=\"";
		for (auto& pt: points)
			ss	<< translateX(pt.x, layout)
				<< ","
				<< translateY(pt.y, layout)
				<< " ";
		ss	<<
			concatenate(
				"\" ",
				fill.toString(layout),
				stroke.toString(layout),
				emptyElemEnd()
				);
		return ss.str();
	}
	void offset(const Point& offset)
	{
		for (auto& pt: points) {
			pt.x += offset.x;
			pt.y += offset.y;
		}
	}
private:
	vector<Point> points;
};

class Polyline : public Shape
{
public:
	Polyline(const Fill& fill = Fill(), const Stroke& stroke = Stroke())
		: Shape(fill, stroke) { }
	Polyline(const Stroke& stroke = Stroke()) : Shape(Color::Transparent, stroke) { }
	Polyline(const vector<Point>& points,
		const Fill& fill = Fill(),
		const Stroke& stroke = Stroke())
		:
		Shape(fill, stroke),
		points(points)
	{ }

	Polyline& operator += (std::initializer_list<double[2]> pts)
	{
		for (auto& pt: pts) {
			points.push_back(Point(pt[0], pt[1]));
		}
		return *this;
	}

	Polyline& operator << (const Point& point)
	{
		points.push_back(point);
		return *this;
	}

	string toString(const Layout& layout) const
	{
		ostringstream ss;
		ss	<< concatenate(
				elemStart("polyline"),
				"points=\""
			);
		for (auto& pt: points) {
			ss	<< translateX(pt.x, layout) << ","
				<< translateY(pt.y, layout) << " ";
		}
		ss	<< concatenate(
				"\" ",
				fill.toString(layout),
				stroke.toString(layout),
				emptyElemEnd()
				);
		return ss.str();
	}
	void offset(const Point& offset)
	{
		for (auto& pt: points) {
			pt.x += offset.x;
			pt.y += offset.y;
		}
	}
	vector<Point> points;
};

class Text : public Shape
{
public:
	Text(const Point& origin,
		const string& content,
		const Fill& fill = Fill(),
		const Font& font = Font(),
		const Stroke& stroke = Stroke()
		)
		:
		Shape(fill, stroke),
		origin(origin),
		content(content),
		font(font)
	{ }
	string toString(const Layout& layout) const
	{
		return concatenate(
			elemStart("text"),
			attribute("x", translateX(origin.x, layout)),
			attribute("y", translateY(origin.y, layout)),
			fill.toString(layout),
			stroke.toString(layout),
			font.toString(layout),
			">",
			content,
			elemEnd("text")
		);
	}
	void offset(const Point& offset)
	{
		origin.x += offset.x;
		origin.y += offset.y;
	}
private:
	Point origin;
	string content;
	Font font;
};

// Sample charting class.
class LineChart : public Shape
{
public:
	LineChart(Dimensions margin = Dimensions(),
		double scale = 1,
		const Stroke& axis_stroke = Stroke(.5, Color::Purple))
		:
		axis_stroke(axis_stroke),
		margin(margin),
		scale(scale)
	{ }
	LineChart& operator << (const Polyline& polyline)
	{
		if (polyline.points.empty())
			return *this;
		polylines.push_back(polyline);
		return *this;
	}
	string toString(const Layout& layout) const
	{
		if (polylines.empty())
			return "";
		string ret;
		for (auto& polyline: polylines) {
			ret += polylineToString(polyline, layout);
		}
		return ret + axisString(layout);
	}
	void offset(const Point& offset)
	{
		for (auto& polyline: polylines) {
			polyline.offset(offset);
		}
	}
private:
	Stroke axis_stroke;
	Dimensions margin;
	double scale;
	vector<Polyline> polylines;

	optional<Dimensions> getDimensions() const
	{
		if (polylines.empty())
			return optional<Dimensions>();

		auto min = getMinPoint(polylines[0].points);
		auto max = getMaxPoint(polylines[0].points);
		for (unsigned i = 1; i < polylines.size(); ++i) {
			auto minPt = getMinPoint(polylines[i].points);
			auto maxPt = getMaxPoint(polylines[i].points);
			if (minPt->x < min->x)	min->x = minPt->x;
			if (minPt->y < min->y)	min->y = minPt->y;
			if (maxPt->x > max->x)	max->x = maxPt->x;
			if (maxPt->y > max->y)	max->y = maxPt->y;
		}

		return make_optional(Dimensions(max->x - min->x, max->y - min->y));
	}
	string axisString(const Layout& layout) const
	{
		optional<Dimensions> dimensions = getDimensions();
		if (!dimensions)
			return "";

		// Make the axis 10% wider and higher than the data points.
		double width = dimensions->width * 1.1;
		double height = dimensions->height * 1.1;

		// Draw the axis.
		Polyline axis(Color::Transparent, axis_stroke);
		axis << Point(margin.width, margin.height + height)
			<< Point(margin.width, margin.height)
			<< Point(margin.width + width, margin.height);

		return axis.toString(layout);
	}
	string polylineToString(const Polyline& polyline, const Layout& layout) const
	{
		Polyline shifted_polyline = polyline;
		shifted_polyline.offset(Point(margin.width, margin.height));

		vector<Circle> vertices;
		for (auto& pt: shifted_polyline.points) {
			vertices.push_back(
				Circle(
					pt,
					getDimensions()->height / 30.0,
					Color::Black
				)
			);
		}
		return shifted_polyline.toString(layout) + vectorToString(vertices, layout);
	}
};

class Document
{
public:
	Document(const string& file_name, Layout layout = Layout())
		:
		file_name(file_name),
		layout(layout)
	{ }

	Document& operator << (const Shape& shape)
	{
		body_nodes_str += shape.toString(layout);
		return *this;
	}
	string toString() const
	{
		return concatenate(
			"<?xml ",
			attribute("version", "1.0"),
			attribute("standalone", "no"),
			"?>\n<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" ",
			"\"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n<svg ",
			attribute("width", layout.dimensions.width, "px"),
			attribute("height", layout.dimensions.height, "px"),
			attribute("xmlns", "http://www.w3.org/2000/svg"),
			attribute("version", "1.1"),
			">\n",
			body_nodes_str,
			elemEnd("svg")
		);
	}
	bool save() const
	{
		std::ofstream ofs(file_name.c_str());
		if (!ofs.good())
			return false;

		ofs << toString();
		ofs.close();
		return true;
	}
private:
	string file_name;
	Layout layout;

	string body_nodes_str;
};

} // namespace svg
