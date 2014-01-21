#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Polygon_2_algorithms.h>
#include <iostream>

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef K::Point_2 Point;

extern "C" int R_IsNA(double); // R_ext/Arith.h

class Shape {
    Point *points;
    int n, nparts;
    std::vector<int> part_vec;

public:
    Shape() {
	delete points;
    }

    Shape(double *x, double *y, int n, int *parts, int parts_n) {
	points = new Point[n];
	this->n = n;
	//part_vec = std::vector<int>(); // # of points in each polygon
	
	if (parts && parts_n == 1) { // one part - easy, no splitting needed
	    for (int i = 0; i < n; i++)
		points[i] = Point(x[i], y[i]);
	    part_vec.push_back(n);
	} else { // go by parts
	    int part_ptr = 1, next_part = n;
	    if (parts)
		next_part = parts[part_ptr];
	    
	    int i, j = 0, k = 0;
	    for (i = 0; i < n; i++) {
		if (i >= next_part) {
		    if (j) {
			part_vec.push_back(j);
			j = 0;
		    }
		    next_part = (++part_ptr >= parts_n) ? n : parts[part_ptr];
		    i--; // go back as to re-process the current point which we didn't add yet
		    continue;
		} else if (R_IsNA(x[i])) {
		    if (j) {
			part_vec.push_back(j);
			j = 0;
		    }
		    while (i < n && R_IsNA(x[i])) // skip over all NAs
			i++;
		    if (i >= n)
			break;
		    i--; // go back, and re-process as well
		    continue;
		}
		points[k++] = Point(x[i], y[i]);
	    }
	    if (j)
		part_vec.push_back(j);
	}

	nparts = part_vec.size();
    }
    
    int inside(double x, double y) {
	int ptr = 0;
	Point pt(x, y);
	for (int i = 0; i < nparts; i++) {
	    int np = part_vec[i];
	    switch(CGAL::bounded_side_2(points + ptr, points + ptr + n, pt, K())) {
	    case CGAL::ON_BOUNDED_SIDE:
		return 1;
	    case CGAL::ON_BOUNDARY:
		return 2;
	    case CGAL::ON_UNBOUNDED_SIDE:
		;
		// carry on, could be in other parts
	    }
	    ptr += np;
	}
	return 0;
    }
};

extern "C" void *load_shape_(double *x, double *y, int n, int *parts, int parts_n) {
    Shape *shape = new Shape(x, y, n, parts, parts_n);
    return (void*) shape;
}

extern "C" int inside_shape_(void *s, double x, double y) {
    Shape *shape = (Shape*) s;
    return shape->inside(x, y);
}

extern "C" void free_shape_(void *s) {
    Shape *shape = (Shape*) s;
    delete shape;
}
