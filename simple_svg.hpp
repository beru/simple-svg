
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
#include <boost/lexical_cast.hpp>

using boost::optional;
using boost::make_optional;
using std::string;
using std::vector;

namespace svg {

// Utility XML/String Functions.
template <typename T>
void attribute(
	string& s,
	const string& attribute_name,
	const T& value,
	const string& unit = "")
{
	s += attribute_name;
	s += "=\"";
	s += toString(value);
	s += unit;
	s += "\" ";
}
void elemStart(string& s, const string& element_name)
{
	s += "\t<";
	s += element_name;
	s += " ";
}
string elemEnd(const string& element_name)
{
	return "</" + element_name + ">\n";
}
string emptyElemEnd()
{
	return "/>\n";
}

template <typename T>
string toString(const T& v)
{
	return boost::lexical_cast<string>(v);
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

	Point& operator += (const Point& p)
	{
		x += p.x;
		y += p.y;
		return *this;
	}
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

template <typename T>
void attribute(
	string& s,
	const string& attribute_name,
	const T& value,
	const Layout& layout)
{
	s += attribute_name;
	s += "=\"";
	value.toString(s, layout);
	s += "\" ";
}

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
	virtual void toString(string& s, const Layout& layout) const = 0;
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
	void toString(string& s, const Layout&) const override
	{
		if (transparent)
			s += "transparent";
		else {
			s += "rgb(";
			s += svg::toString(red);
			s += ",";
			s += svg::toString(green);
			s += ",";
			s += svg::toString(blue);
			s += ")";
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
	void toString(string& s, const Layout& layout) const override
	{
		attribute(s, "fill", color, layout);
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
	void toString(string& s, const Layout& layout) const override
	{
		// If stroke width is invalid.
		if (width < 0)
			return;
		attribute(s, "stroke-width", translateScale(width, layout));
		attribute(s, "stroke", color, layout);
	}
private:
	double width;
	Color color;
};

class Font : public Serializeable
{
public:
	Font(double size = 12, const string& family = "Verdana") : size(size), family(family) { }
	void toString(string& s, const Layout& layout) const override
	{
		attribute(s, "font-size", translateScale(size, layout));
		attribute(s, "font-family", family);
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
	virtual void toString(string& s, const Layout& layout) const = 0;
	virtual void offset(const Point& offset) = 0;
protected:
	Fill fill;
	Stroke stroke;
};

template <typename T>
void vectorToString(string& s, const vector<T>& collection, const Layout& layout)
{
	for (auto& val: collection) {
		val.toString(s, layout);
	}
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
	void toString(string& s, const Layout& layout) const override
	{
		elemStart(s, "circle");
		attribute(s, "cx", translateX(center.x, layout));
		attribute(s, "cy", translateY(center.y, layout));
		attribute(s, "r", translateScale(radius, layout));
		fill.toString(s, layout);
		stroke.toString(s, layout);
		s += emptyElemEnd();
	}
	void offset(const Point& offset)
	{
		center += offset;
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
	void toString(string& s, const Layout& layout) const override
	{
		elemStart(s, "ellipse");
		attribute(s, "cx", translateX(center.x, layout));
		attribute(s, "cy", translateY(center.y, layout));
		attribute(s, "rx", translateScale(radius_width, layout));
		attribute(s, "ry", translateScale(radius_height, layout));
		fill.toString(s, layout);
		stroke.toString(s, layout);
		s += emptyElemEnd();
	}
	void offset(const Point& offset)
	{
		center += offset;
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
	void toString(string& s, const Layout& layout) const override
	{
		elemStart(s, "rect");
		attribute(s, "x", translateX(edge.x, layout));
		attribute(s, "y", translateY(edge.y, layout));
		attribute(s, "width", translateScale(width, layout));
		attribute(s, "height", translateScale(height, layout));
		fill.toString(s, layout);
		stroke.toString(s, layout);
		s += emptyElemEnd();
	}
	void offset(const Point& offset)
	{
		edge += offset;
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
	void toString(string& s, const Layout& layout) const override
	{
		elemStart(s, "line");
		attribute(s, "x1", translateX(start_point.x, layout));
		attribute(s, "y1", translateY(start_point.y, layout));
		attribute(s, "x2", translateX(end_point.x, layout));
		attribute(s, "y2", translateY(end_point.y, layout));
		stroke.toString(s, layout);
		s += emptyElemEnd();
	}
	void offset(const Point& offset)
	{
		start_point += offset;
		end_point += offset;
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
	void toString(string& s, const Layout& layout) const override
	{
		elemStart(s, "polygon");
		s += "points=\"";
		for (auto& pt: points) {
			s += svg::toString(translateX(pt.x, layout));
			s += ",";
			s += svg::toString(translateY(pt.y, layout));
			s += " ";
		}
		s += "\" ";
		fill.toString(s, layout);
		stroke.toString(s, layout);
		s += emptyElemEnd();
	}
	void offset(const Point& offset)
	{
		for (auto& pt: points) {
			pt += offset;
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

	void toString(string& s, const Layout& layout) const override
	{
		elemStart(s, "polyline");
		s += "points=\"";
		for (auto& pt: points) {
			s += svg::toString(translateX(pt.x, layout));
			s += ",";
			s += svg::toString(translateY(pt.y, layout));
			s += " ";
		}
		s += "\" ";
		fill.toString(s, layout);
		stroke.toString(s, layout);
		s += emptyElemEnd();
	}
	void offset(const Point& offset)
	{
		for (auto& pt: points) {
			pt += offset;
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
	void toString(string& s, const Layout& layout) const override
	{
		elemStart(s, "text");
		attribute(s, "x", translateX(origin.x, layout));
		attribute(s, "y", translateY(origin.y, layout));
		fill.toString(s, layout);
		stroke.toString(s, layout);
		font.toString(s, layout);
		s += ">";
		s += content;
		s += elemEnd("text");
	}
	void offset(const Point& offset)
	{
		origin += offset;
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
	void toString(string& s, const Layout& layout) const override
	{
		if (polylines.empty())
			return;
		for (auto& polyline: polylines) {
			polylineToString(s, polyline, layout);
		}
		axisString(s, layout);
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
	void axisString(string& s, const Layout& layout) const
	{
		optional<Dimensions> dimensions = getDimensions();
		if (!dimensions)
			return;

		// Make the axis 10% wider and higher than the data points.
		double width = dimensions->width * 1.1;
		double height = dimensions->height * 1.1;

		// Draw the axis.
		Polyline axis(Color::Transparent, axis_stroke);
		axis << Point(margin.width, margin.height + height)
			<< Point(margin.width, margin.height)
			<< Point(margin.width + width, margin.height);

		axis.toString(s, layout);
	}
	void polylineToString(string& s, const Polyline& polyline, const Layout& layout) const
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
		shifted_polyline.toString(s, layout);
		vectorToString(s, vertices, layout);
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
		shape.toString(body_nodes_str, layout);
		return *this;
	}
	void toString(string& s) const
	{
		s += "<?xml ";
		attribute(s, "version", "1.0");
		attribute(s, "standalone", "no");
		s += "?>\n<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" ";
		s += "\"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n<svg ";
		attribute(s, "width", layout.dimensions.width, "px");
		attribute(s, "height", layout.dimensions.height, "px");
		attribute(s, "xmlns", "http://www.w3.org/2000/svg");
		attribute(s, "version", "1.1");
		s += ">\n";
		s += body_nodes_str;
		s += elemEnd("svg");
	}
	bool save() const
	{
		FILE* f = fopen(file_name.c_str(), "wb");
		if (!f) {
			return false;
		}
		string s;
		toString(s);
		fwrite(s.c_str(), 1, s.size(), f);
		fclose(f);
		return true;
	}
private:
	string file_name;
	Layout layout;

	string body_nodes_str;
};

} // namespace svg
