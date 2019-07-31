// Standard Perlin-noise implementation
// Copyright (C) 2019 Tomasz Dobrowolski
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include <cstdint>
#include <array>
#include <cmath>

int32_t crand(uint32_t& seed) {
  seed = seed * 214013 + 2531011;
  return static_cast<int32_t>((seed >> 16) & 0x7fff);
}

inline float fgrad(uint32_t h, float x, float y, float z)
{
    switch (h&15)
    {
        case  0: return( x+y  );
        case  1: return(-x+y  );
        case  2: return( x-y  );
        case  3: return(-x-y  );
        case  4: return( x  +z);
        case  5: return(-x  +z);
        case  6: return( x  -z);
        case  7: return(-x  -z);
        case  8: return(   y+z);
        case  9: return(  -y+z);
        case 10: return(   y-z);
        case 11: return(  -y-z);
        case 12: return( x+y  );
        case 13: return(-x+y  );
        case 14: return(   y-z);
        case 15: return(  -y-z);
    }
    return(0);
}

inline uint32_t floormod256(float x)
{
    // -1 => 255
    // -255 => 1
    // -256 => 0
    return (x < 0)
        ? 255 - (static_cast<uint32_t>(-x - 1) & 255)
        : (static_cast<uint32_t>(x) & 255);
}

using NoiseLUT = std::array<uint32_t, 768>;

static NoiseLUT gen_table()
{
    NoiseLUT arr;
    uint32_t i;
    uint32_t seed = 1;
    for(i=0; i<256; ++i) arr[i] = i;
    for(i=256-1;i> 0;i--) {
        uint32_t j = ((static_cast<uint32_t>(crand(seed))*(i+1)) >> 15);
        std::swap(arr[i], arr[j]);
    }
    for(i=0; i<256; ++i) {
        arr[i+256] = arr[i];
        arr[i+512] = arr[i];
    }
    return arr;
}

static NoiseLUT noisep = gen_table();

float noise1d(float fx)
{
    uint32_t l0;
    float p0, t0, f0, f1;

    p0 = std::floor(fx);
    fx -= p0; t0 = (3.0f - 2.0f*fx)*fx*fx;
    l0 = floormod256(p0);
    f0 = fgrad(noisep[noisep[noisep[l0  ]]],fx  ,0,0);
    f1 = fgrad(noisep[noisep[noisep[l0+1]]],fx-1,0,0);
    return ((f1-f0)*t0 + f0);
}

float noise2d(float fx, float fy)
{
    uint32_t i, l0, l1, a0, a1, a2, a3;
    float p0, p1, t0, t1, f0, f1, f2, f3;

    p0 = std::floor(fx);
    p1 = std::floor(fy);
    fx -= p0; t0 = (3.0f - 2.0f*fx)*fx*fx;
    fy -= p1; t1 = (3.0f - 2.0f*fy)*fy*fy;
    l0 = floormod256(p0);
    l1 = floormod256(p1);
    i = noisep[l0  ]; a0 = noisep[i+l1]; a2 = noisep[i+l1+1];
    i = noisep[l0+1]; a1 = noisep[i+l1]; a3 = noisep[i+l1+1];
    f0 = fgrad(noisep[a0],fx  ,fy,0);
    f1 = fgrad(noisep[a1],fx-1,fy,0); fy--;
    f2 = fgrad(noisep[a2],fx  ,fy,0);
    f3 = fgrad(noisep[a3],fx-1,fy,0);
    f0 = (f1-f0)*t0 + f0;
    f1 = (f3-f2)*t0 + f2;
    return((f1-f0)*t1 + f0);
}

float noise3d(float fx, float fy, float fz)
{
    uint32_t i, l0, l1, l2, a0, a1, a2, a3;
    float p0, p1, p2, t0, t1, t2, f0, f1, f2, f3, f4, f5, f6, f7;

    p0 = std::floor(fx);
    p1 = std::floor(fy);
    p2 = std::floor(fz);
    fx -= p0; t0 = (3.0f - 2.0f*fx)*fx*fx;
    fy -= p1; t1 = (3.0f - 2.0f*fy)*fy*fy;
    fz -= p2; t2 = (3.0f - 2.0f*fz)*fz*fz;
    l0 = floormod256(p0);
    l1 = floormod256(p1);
    l2 = floormod256(p2);
    i = noisep[l0  ]; a0 = noisep[i+l1]; a2 = noisep[i+l1+1];
    i = noisep[l0+1]; a1 = noisep[i+l1]; a3 = noisep[i+l1+1];
    f0 = fgrad(noisep[a0+l2  ],fx  ,fy  ,fz);
    f1 = fgrad(noisep[a1+l2  ],fx-1,fy  ,fz);
    f2 = fgrad(noisep[a2+l2  ],fx  ,fy-1,fz);
    f3 = fgrad(noisep[a3+l2  ],fx-1,fy-1,fz); fz--;
    f4 = fgrad(noisep[a0+l2+1],fx  ,fy  ,fz);
    f5 = fgrad(noisep[a1+l2+1],fx-1,fy  ,fz);
    f6 = fgrad(noisep[a2+l2+1],fx  ,fy-1,fz);
    f7 = fgrad(noisep[a3+l2+1],fx-1,fy-1,fz);
    f0 = (f1-f0)*t0 + f0;
    f1 = (f3-f2)*t0 + f2;
    f2 = (f5-f4)*t0 + f4;
    f3 = (f7-f6)*t0 + f6;
    f0 = (f1-f0)*t1 + f0;
    f1 = (f3-f2)*t1 + f2;
    return((f1-f0)*t2 + f0);
}
