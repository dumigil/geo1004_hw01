#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <filesystem>
namespace fs = std::filesystem;

#include "Point.h"
#include "Rows.h"
#include "VoxelGrid.h"

float signed_volume(const Point &a, const Point &b, const Point &c, const Point &d) {
  // to do
}

bool intersects(const Point &orig, const Point &dest, const Point &v0, const Point &v1, const Point &v2) {
  // to do
}
bool read_obj(std::string filepath){
    std::ifstream infile(filepath.c_str(),std::ifstream::in);
    if(!infile){
        std::cerr<<"Input file not found.\n";
        return false;
    }
    std::string cursor;ls
    

}


int main(int argc, const char * argv[]) {
  const char *file_in = "bag_bk.obj";
  const char *file_out = "vox.obj";
  float voxel_size = 1.0;

  // Read file
  std::vector<Point> vertices;
  std::vector<std::vector<unsigned int>> faces;



  // to do
  std::cout<<"It is working"<<std::endl;
  // Create grid
  Rows rows;
  // to do
  VoxelGrid voxels(rows.x, rows.y, rows.z);
  
  // Voxelise
  for (auto const &triangle: faces) {
    // to do
  }
  
  // Fill model
  // to do
  
  // Write voxels
  // to do
  
  return 0;
}
