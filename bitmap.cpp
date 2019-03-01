#include <fstream>

#include "bitmap.h"


Bitmap::Bitmap(const std::string &file) : dx {0}, dy {0}, bitcount {0}, r {0}, g {0}, b {0}
{
  std::ifstream iFile(file);
  if (iFile.bad()) return;

  // Tag
  if (iFile.get() != 'B' || iFile.get() != 'M') return;

  // Saltarse Header
  iFile.ignore(12);

  // Info-Header
  iFile.ignore(4);

  dx = iFile.get();
  dx += iFile.get() << 8;
  iFile.ignore(2);
  dy = iFile.get();
  dy += iFile.get() << 8;
  iFile.ignore(2);

  r = new unsigned char[dx * dy];
  g = new unsigned char[dx * dy];
  b = new unsigned char[dx * dy];

  iFile.ignore(28);

  for(int i = dy - 1; i >= 0; i--) {
    for(int j = 0; j < dx; j++) {
      b[i * dx + j] = iFile.get();
      g[i * dx + j] = iFile.get();
      r[i * dx + j] = iFile.get();
    }
    if ((dx * 3) % 4 != 0) {
      iFile.ignore(4 - (dx * 3) % 4);
    }
  }
}


Bitmap::~Bitmap()
{
  delete []r;
  delete []g;
  delete []b;
}
