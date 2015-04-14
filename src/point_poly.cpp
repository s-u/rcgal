#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Polygon_2_algorithms.h>
#include <CGAL/Polygon_2.h>

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef K::Point_2 Point;
typedef CGAL::Polygon_2<K> Polygon;

extern "C" int R_IsNA(double); // R_ext/Arith.h

class Shape {
    // a shape is a list of polygons
    // polygons are clockwise; counter-clockwise polygons are holes and
    // must be contained within the outer polygon (they may touch vertices, though)
    std::vector<Polygon> polygons;
    bool has_holes;

public:
    Shape(double *x, double *y, int n, int *parts, int parts_n) {
	bool holes = false;
	if (n > 0) {
	    Polygon poly;
	    
	    if (parts && parts_n == 1) { // one part - easy, no splitting needed
		for (int i = 0; i < n; i++)
		    poly.push_back(Point(x[i], y[i]));
		if (poly.orientation() == CGAL::COUNTERCLOCKWISE)
		    holes = true;
		polygons.push_back(poly);
	    } else { // go by parts
		int part_ptr = 1, next_part = n;
		if (parts)
		    next_part = parts[part_ptr];
		
		int i;
		for (i = 0; i < n; i++) {
		    if (i >= next_part) {
			if (poly.size()) {
			    polygons.push_back(poly);
			    if (poly.orientation() == CGAL::COUNTERCLOCKWISE)
				holes = true;
			    poly.clear();
			}
			next_part = (++part_ptr >= parts_n) ? n : parts[part_ptr];
			i--; // go back as to re-process the current point which we didn't add yet
			continue;
		    } else if (R_IsNA(x[i])) {
			if (poly.size()) {
			    polygons.push_back(poly);
			    if (poly.orientation() == CGAL::COUNTERCLOCKWISE)
				holes = true;
			    poly.clear();
			}
			while (i < n && R_IsNA(x[i])) // skip over all NAs
			    i++;
			if (i >= n)
			    break;
			i--; // go back, and re-process as well
			continue;
		    }
		    poly.push_back(Point(x[i], y[i]));
		}
		if (poly.size()) {
		    polygons.push_back(poly);
		    if (poly.orientation() == CGAL::COUNTERCLOCKWISE)
			holes = true;
		}
	    }
	}	    
	has_holes = holes;
    }

    int count() {
	return polygons.size();
    }
    
    void orientations(int *where) {
	int n = polygons.size();
	for (int i = 0; i < n; i++)
	    where[i] = (int) polygons[i].orientation();
    }

    void areas(double *where) {
	int n = polygons.size();
	for (int i = 0; i < n; i++)
	    where[i] = (double) polygons[i].area();
    }

    int inside(double x, double y) {
	int nparts = polygons.size();
	Point pt(x, y);
	if (!has_holes) // no holes - simply check for any match
	    for (int i = 0; i < nparts; i++) {
		switch(polygons[i].bounded_side(pt)) {
		case CGAL::ON_BOUNDARY:
		    return 2;
		case CGAL::ON_BOUNDED_SIDE:
		    return 1;
		case CGAL::ON_UNBOUNDED_SIDE:
		    ;
		    // carry on, could be in other parts
		}
	    }
	else { // holes - we have to check the orientation
	    int inside_count = 0; // we have to count islands - holes
	    for (int i = 0; i < nparts; i++) {
		bool is_hole = polygons[i].orientation() == CGAL::COUNTERCLOCKWISE;
		switch(polygons[i].bounded_side(pt)) {
		case CGAL::ON_BOUNDARY:
		    return 2;
		case CGAL::ON_BOUNDED_SIDE:
		    if (is_hole) inside_count--; else inside_count++;
		    break;
		case CGAL::ON_UNBOUNDED_SIDE:
		    ;
		}
	    }
	    return (inside_count > 0) ? 1 : 0;
	}
	return 0; // superfluous ... jsut for dumb compilers
    }
};

//---- C API ----

extern "C" void *shape_load_(double *x, double *y, int n, int *parts, int parts_n) {
    Shape *shape = new Shape(x, y, n, parts, parts_n);
    return (void*) shape;
}

extern "C" int shape_inside_(void *s, double x, double y) {
    Shape *shape = (Shape*) s;
    return shape->inside(x, y);
}

extern "C" void shape_orientations_(void *s, int *where) {
    Shape *shape = (Shape*) s;
    shape->orientations(where);
}

extern "C" void shape_areas_(void *s, double *where) {
    Shape *shape = (Shape*) s;
    shape->areas(where);
}

extern "C" int shape_count_(void *s) {
    Shape *shape = (Shape*) s;
    return shape->count();
}    

extern "C" void shape_free_(void *s) {
    Shape *shape = (Shape*) s;
    delete shape;
}
