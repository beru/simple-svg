
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

#include "simple_svg.hpp"
#include "timer.h"

using namespace svg;

// Demo page shows sample usage of the Simple SVG library.

void demo()
{
	Dimensions dimensions(100, 100);
	Document doc("my_svg.svg", Layout(dimensions, Layout::BottomLeft));

	// Red image border.
	Polygon border(Stroke(1, Color::Red));
	border
		<< Point(0, 0)
		<< Point(dimensions.width, 0)
		<< Point(dimensions.width, dimensions.height)
		<< Point(0, dimensions.height);
	doc << border;

	// Long notation.  Local variable is created, children are added to varaible.
	LineChart chart(5.0);
	Polyline polyline_a(Stroke(.5, Color::Blue));
	Polyline polyline_b(Stroke(.5, Color::Aqua));
	Polyline polyline_c(Stroke(.5, Color::Fuchsia));
	polyline_a += {
		{0, 0},
		{10, 30},
		{20, 40},
		{30, 45},
		{40, 44},
	};
	polyline_b += {
		{0, 10},
		{10, 22},
		{20, 30},
		{30, 32},
		{40, 30},
	};
	polyline_c += {
		{0, 12},
		{10, 15},
		{20, 14},
		{30, 10},
		{40, 2},
	};
	chart
		<< polyline_a
		<< polyline_b
		<< polyline_c;
	doc << chart;

	// Condensed notation, parenthesis isolate temporaries that are inserted into parents.
	doc << (
		LineChart(Dimensions(65, 5))
		<< (Polyline(Stroke(.5, Color::Blue)) += {{0, 0}, {10, 8}, {20, 13}})
		<< (Polyline(Stroke(.5, Color::Orange)) += {{0, 10}, {10, 16}, {20, 20}})
		<< (Polyline(Stroke(.5, Color::Cyan)) += {{0, 5}, {10, 13}, {20, 16}})
		);

	doc << Circle(
			Point(80, 80),						// center
			20,									// diameter
			Fill(Color(100, 200, 120)),			// fill
			Stroke(1, Color(200, 250, 150))		// stroke
		);

	doc << Text(
			Point(5, 77),		// origin
			"Simple SVG",		// content
			Color::Silver,		// fill color
			Font(10, "Verdana")	// font
		);

	doc << (
			Polygon(
				Color(200, 160, 220),
				Stroke(.5, Color(150, 160, 200))
			)
			<< Point(20, 70)
			<< Point(25, 72)
			<< Point(33, 70)
			<< Point(35, 60)
			<< Point(25, 55)
			<< Point(18, 63)
		);

	doc << Rectangle(
			Point(70, 55),	// edge
			20,				// width
			15,				// height
			Color::Yellow	// fill color
		);

	doc.save();
}

int main()
{
	Timer t;
	demo();
	printf("%f\n", t.ElapsedSecond() * 1000.0);
	return 0;
}

