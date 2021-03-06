#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <iterator>
#include <algorithm>
#include <Points.h>
#include <vector>
#include <list>
#include "cmath"
#include "map"
#include "stack"
#include <iomanip>
#include "KDTree/sample.cpp"
#include "KDTree/kdtree.h"
#define CONVHULL_3D_ENABLE
#include "convhull_3d/convhull_3d.h"
#include <CGAL/convex_hull_3.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <boost/uuid/uuid.hpp>            // uuid class
#include <boost/uuid/uuid_generators.hpp> // generators
#include <boost/uuid/uuid_io.hpp>         // streaming operators etc.



typedef CGAL::Exact_predicates_exact_constructions_kernel   K;
typedef CGAL::Polyhedron_3<K>                               Polyhedron_3;
typedef K::Point_3                                          Point_3;
typedef K::Segment_3                                        Segment_3;
typedef K::Triangle_3                                       Triangle_3;
typedef CGAL::Surface_mesh<Point_3>                         Surface_Mesh;
typedef Polyhedron_3::Facet_iterator                        Facet_iterator;
typedef Polyhedron_3::Halfedge_around_facet_circulator      Halfedge_facet_circulator;
typedef Polyhedron_3::Vertex_iterator                       Vertex_iterator;

std::vector<Point> _input_points;

void read_ply (const char *file_in) {
    std::ifstream infile(file_in, std::ifstream::in);
    if (!infile)
    {
        std::cerr << "Input file not found.\n";
    }
    std::string cursor;
    // start reading the header
    std::getline(infile, cursor);
    if (cursor != "ply") {
        std::cerr << "Magic ply keyword not found\n";
    }
    std::getline(infile, cursor);
    if (cursor != "format ascii 1.0") {
        std::cerr << "Incorrect ply format\n";
    }
    // read the remainder of the header
    std::string line = "";
    int vertex_count = 0;
    bool expectVertexProp = false, foundNonVertexElement = false;
    int property_count = 0;
    std::vector<std::string> property_names;
    int pos_x = -1, pos_y = -1, pos_z = -1, pos_segment_id = -1;

    while (line != "end_header") {
        std::getline(infile, line);
        std::istringstream linestream(line);

        linestream >> cursor;
        // read vertex element and properties
        if (cursor == "element") {
            linestream >> cursor;
            if (cursor == "vertex") {
                // check if this is the first element defined in the file. If not exit the function
                if (foundNonVertexElement) {
                    std::cerr << "vertex element is not the first element\n";
                }

                linestream >> vertex_count;
                expectVertexProp = true;
            } else {
                foundNonVertexElement = true;
            }
        } else if (expectVertexProp) {
            if (cursor != "property") {
                expectVertexProp = false;
            } else {
                // read property type
                linestream >> cursor;
                if (cursor.find("float") != std::string::npos || cursor == "double") {
                    // read property name
                    linestream >> cursor;
                    if (cursor == "x") {
                        pos_x = property_count;
                    } else if (cursor == "y") {
                        pos_y = property_count;
                    } else if (cursor == "z") {
                        pos_z = property_count;
                    }
                    ++property_count;
                }
                else if (cursor.find("uint") != std::string::npos || cursor == "int") {
                    // read property name
                    linestream >> cursor;
                    if (cursor == "segment_id") {
                        pos_segment_id = property_count;
                    }
                    ++property_count;
                }
            }

        }
    }

    // check if we were able to locate all the coordinate properties
    if ( pos_x == -1 || pos_y == -1 || pos_z == -1) {
        std::cerr << "Unable to locate x, y and z vertex property positions\n";
    }
    // read the vertex properties
    for (int vi = 0; vi < vertex_count; ++vi) {
        std::getline(infile, line);
        std::istringstream linestream(line);

        double x{},y{},z{};
        int sid{};
        for (int pi = 0; pi < property_count; ++pi) {
            linestream >> cursor;
            if ( pi == pos_x ) {
                x = std::stod(cursor);
            } else if ( pi == pos_y ) {
                y = std::stod(cursor);
            } else if ( pi == pos_z ) {
                z = std::stod(cursor);
            } else if ( pi == pos_segment_id ) {
                sid = std::stoi(cursor);
            }
        }
        auto p = Point{x, y, z};
//        if (pos_segment_id!=-1) {
//            p.segment_id = sid;
//        }
        _input_points.push_back(p);
    }
    std::cout << "Number of points read from .ply file: " << _input_points.size() << std::endl;
}

void read_xyz (const char *file_in) {
    std::ifstream infile(file_in, std::ifstream::in);
    if(!infile){
        std::cerr<<"Input file not found\n";
        return;
    }
    infile.ignore(1000,'\n');
    std::string line= "";
    while (std::getline(infile, line)) {
        std::stringstream ss;
        ss.clear();
        ss.str(line);
        float x, y, z;
        ss >> x >> y >> z;
        Point p={x,y,z};
        _input_points.push_back(p);
    }
    std::cout<<"n0 of points read from .xyz file : "<<_input_points.size()<<std::endl;
}


std::vector<std::vector<Point>>Trees;
void ClusterTrees(std::vector<Point> & _input_points) {

    //make KDtree
    std::vector<MyPoint> points(_input_points.size());
    for (int i = 0; i<_input_points.size(); i++){
        const double x = _input_points[i].x;
        const double y = _input_points[i].y;
        const double z = _input_points[i].z;
        points[i] = MyPoint(x, y, z);
    }
    kdt::KDTree<MyPoint> kdtree(points);
    //startClustering

    std::vector<Point> tree;
    std::vector<int> tp;
//    std::list<int> checked;
    int id=0;
    for (const auto &p:_input_points) {
        std::stack<Point> s;
//        std::cout<<p.x<<" "<<p.y<<" "<<p.z<<" "<<_input_points[id].x<<" "<<_input_points[id].y<<" "<<_input_points[id].z<<std::endl;
        if (p.segment_id==0) {
            tree.clear();
            tp.clear();
            tp.push_back(id);
            tree.push_back(p);
            s.push(p);
            while (!s.empty()) {
                Point pc = s.top();
                s.pop();
                const double x = pc.x;
                const double y = pc.y;
                const double z = pc.z;
                MyPoint current = MyPoint(x, y, z);
                std::vector<int> neighbours = kdtree.radiusSearch(current, 2);
//                std::cout<<neighbours.size();
                if (neighbours.size()>=3){
                    for(auto const n:neighbours){
                        if(std::find(tp.begin(), tp.end(), n) != tp.end()){continue;}
                        if(_input_points[n].segment_id==0){continue;}
                        tp.push_back(n);
                        tree.push_back(_input_points[n]);
                        s.push(_input_points[n]);
                        _input_points[n].segment_id=1;
                    }
                }
            }
            _input_points[id].segment_id=1;

            if (tree.size()>12){
                //std::cout<<tree.size()<<std::endl;
                Trees.push_back(tree);}
        }
        ++id;
    }
    //std::cout<<"NUMBER OF TREES "<<Trees.size();
}

void convexHull(std::vector<std::vector<Point>> pointList, std::string out){
    std::ofstream outfile(out, std::ios::out);
    std::vector<Point_3> _vertices;
    std::vector<std::vector<Point_3>> cgalTrees;
    std::vector<Polyhedron_3> polyTrees;
    for(const auto &all: pointList){
        std::vector<Point_3> cgalTree;
        for(const auto &p: all){
            Point_3 pt=Point_3(p.x, p.y, p.z);
            cgalTree.push_back(pt);
        }
        cgalTrees.push_back(cgalTree);
    }
    std::vector<Polyhedron_3 > smTrees;
    std::vector<std::vector<std::vector<int>>> treeColl;
    int ct = 0;
    std::string delim2="";
    outfile<<"{\n";
    outfile<<"  \"type\": \"CityJSON\",\n";
    outfile<<"  \"version\": \"1.0\",\n";
    outfile<<"  \"metadata\": {\n";
    outfile<<"      \"referenceSystem\": \"urn:ogc:def:crs:EPSG::7415\"\n";
    outfile<<"                },\n";
    outfile<<"  \"CityObjects\": {\n";


    for(const auto &tree: cgalTrees){
        std::vector<std::vector<int>> intTrees;
        std::vector<Point_3 > pts;
        Polyhedron_3 poly;
        CGAL::convex_hull_3(tree.begin(), tree.end(),poly);

        //std::cout << "The convex hull contains " << poly.size_of_vertices() << " vertices" << std::endl;
        for(Vertex_iterator v = poly.vertices_begin();v!=poly.vertices_end();++v){
            pts.push_back(v->point());
        }


        Point_3 centerz = *std::min_element(pts.begin(), pts.end(), [](const Point_3 &a, const Point_3 &b){
            return a.z() < b.z();
        });
        Point_3 min_y = *std::min_element(pts.begin(), pts.end(), [](const Point_3 &a, const Point_3 &b){
            return a.y() < b.y();
        });
        Point_3 max_y = *std::min_element(pts.begin(), pts.end(), [](const Point_3 &a, const Point_3 &b){
            return a.y() > b.y();
        });
        Point_3 min_x = *std::min_element(pts.begin(), pts.end(), [](const Point_3 &a, const Point_3 &b){
            return a.x() < b.x();
        });
        Point_3 max_x = *std::min_element(pts.begin(), pts.end(), [](const Point_3 &a, const Point_3 &b){
            return a.x() > b.x();
        });
        auto centerx = (min_x.x() + max_x.x()) /2;
        auto centery = (min_y.y() + max_y.y()) /2;
        Point_3 center(centerx, centery, centerz.z());
        auto diam_y = max_y.y()-min_y.y();
        auto diam_x = max_x.x()-min_x.x();
        auto diam = (diam_x + diam_y )/2;
        auto trunk = diam/8;
        auto ground = 0;


        Polyhedron_3 poly_new;
        CGAL::convex_hull_3(pts.begin(), pts.end(),poly_new);
        smTrees.push_back(poly_new);


        for(Facet_iterator i = poly_new.facets_begin();i!=poly_new.facets_end();i++){
            std::vector<int> idx;
            Halfedge_facet_circulator j = i->facet_begin();
            CGAL_assertion(CGAL::circulator_size(j) >=3);
            do{
                int idxc = std::distance(poly_new.vertices_begin(), j->vertex());
                idx.push_back(idxc +ct);
            }while(++j != i->facet_begin());
            intTrees.push_back(idx);
        }
        treeColl.push_back(intTrees);
        for(Vertex_iterator v = poly_new.vertices_begin();v!=poly_new.vertices_end();++v){
            _vertices.push_back(v->point());
            ct++;
        }
        Point_3 p1((center.x()+(trunk/2)),center.y()+(trunk/2),center.z());
        Point_3 p2((center.x()+(trunk/2)),center.y()-(trunk/2),center.z());
        Point_3 p3((center.x()-(trunk/2)),center.y()-(trunk/2),center.z());
        Point_3 p4((center.x()-(trunk/2)),center.y()+(trunk/2),center.z());

        Point_3 p5((center.x()+(trunk/2)),center.y()+(trunk/2),ground);
        Point_3 p6((center.x()+(trunk/2)),center.y()-(trunk/2),ground);
        Point_3 p7((center.x()-(trunk/2)),center.y()-(trunk/2),ground);
        Point_3 p8((center.x()-(trunk/2)),center.y()+(trunk/2),ground);
        ct--;
        std::vector<int> f1 = {1+ct, 2+ct, 3+ct, 4+ct};
        std::vector<int> f2 = {1+ct, 4+ct, 8+ct, 5+ct};
        std::vector<int> f3 = {1+ct, 5+ct, 6+ct, 2+ct};
        std::vector<int> f4 = {3+ct, 2+ct, 6+ct, 7+ct};
        std::vector<int> f5 = {4+ct, 3+ct, 7+ct, 8+ct};
        std::vector<int> f6 = {5+ct, 8+ct, 7+ct, 6+ct};
        intTrees.push_back(f1);
        intTrees.push_back(f2);
        intTrees.push_back(f3);
        intTrees.push_back(f4);
        intTrees.push_back(f5);
        intTrees.push_back(f6);

        _vertices.push_back(p1);
        _vertices.push_back(p2);
        _vertices.push_back(p3);
        _vertices.push_back(p4);
        _vertices.push_back(p5);
        _vertices.push_back(p6);
        _vertices.push_back(p7);
        _vertices.push_back(p8);
        ct+=9;






        boost::uuids::uuid uuid = boost::uuids::random_generator()();

        outfile << "      " << delim2 <<"\""<< uuid <<"\""<< ": {\n";
        outfile << "          \"type\": \"SolitaryVegetationObject\",\n";
        outfile << "          \"attributes\": {\n";
        outfile << "              \"trunkDiameter\": " << trunk << " ,\n";
        outfile << "              \"crownDiameter\": " << diam << " \n";
        outfile << "          },\n";
        outfile << "          \"geometry\": [{\n";
        outfile << "              \"type\": \"MultiSurface\",\n";
        outfile << "              \"lod\": 1,\n";
        outfile << "              \"boundaries\": [\n";
        std::string delim;
        std::string comma;
        std::string delim3=" ";

        for(auto tree:intTrees) {
            outfile << "                  " << delim3 << "[[";
            std::string comma3=" ";

            for (auto &v: tree) {
                outfile << comma3 << v;
                comma3 = ", ";
            }
            outfile << "]]\n ";
            delim3=", ";

        }
        outfile << "               ]\n";
        outfile << "          }]\n";

        outfile << "          }"<<"\n";
        delim2=",";
    }


    outfile<<"  },\n";
    outfile<<"  \"vertices\": [\n";
    std::string delim4;
    delim4="";
    for(auto &e: _vertices){
        outfile<<std::setprecision(10)<<"    "<<delim4<<"[ "<<e.x()<<", "<<e.y()<<", "<<e.z()<<"]\n";
        delim4=",";
    }
    outfile<<"  ]\n";
    outfile<<"}\n";
    outfile.close();
/*

    int ctr = 0;
    for(const auto &all: _vertices){
        outfile <<"v "<< all.x()<<" "<<all.y()<<" "<<all.z()<<"\n";
    }
    for(const auto &a: treeColl){
        outfile<<"o "<<ctr<<std::endl;
        for(const auto &b: a){
            outfile<<"f ";
            for(const auto &c : b){
                outfile << c<<" ";
            }
            outfile<<std::endl;
        }
        ctr++;
    }
*/
}




int main(int argc, const char * argv[]) {
    const char *file_in = "../LAS.xyz";
    std::string file_out = "../trees_35th.json";
//    read_ply(file_in);
    read_xyz(file_in);
    ClusterTrees(_input_points);
    convexHull(Trees, file_out);
    return 0;
}