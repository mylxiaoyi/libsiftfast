// exact C++ implementation of lowe's sift program
// author: zerofrog(@gmail.com), Sep 2008
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//Lesser GNU General Public License for more details.
//
//You should have received a copy of the GNU Lesser General Public License
//along with this program.  If not, see <http://www.gnu.org/licenses/>.

// This source code was carefully calibrated to match David Lowe's SIFT features program
#include <siftfast.h>

#include <sys/timeb.h>    // ftime(), struct timeb
#include <sys/time.h>

typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

inline u64 GetMicroTime()
{
    struct timeval t;
    gettimeofday(&t, NULL);
    return (u64)t.tv_sec*1000000+t.tv_usec;
}

int main(int argc, char **argv)
{
    Image image = ReadPGM(stdin);
    Keypoint keypts;
    float fproctime;

    cerr << "Finding keypoints..." << endl;
    {
        u64 basetime = GetMicroTime();
        keypts = GetKeypoints(image);
        fproctime = (GetMicroTime()-basetime)*0.000001f;
    }

    // write the keys to the output
    int numkeys = 0;
    Keypoint key = keypts;
    while(key) {
        numkeys++;
        key = key->next;
    }

    cerr << numkeys << " keypoints found in " << fproctime << " seconds." << endl;

    cout << numkeys << " " << 128 << endl;
    key = keypts;
    while(key) {
        cout << key->row << " " << key->col << " " << key->scale << " " << key->ori << endl;

        for(int i = 0; i < 128; ++i) {
            int intdesc = (int)(key->descrip[i]*512.0f);
            assert( intdesc >= 0 );
            
            if( intdesc > 255 )
                intdesc = 255;
            cout << intdesc << " ";
            if( (i&15)==15 )
                cout << endl;
        }
        cout << endl;
        key = key->next;
    }

    FreeKeypoints(keypts);
    DestroyAllResources();

    return 0;
}
