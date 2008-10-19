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
#include "siftfast.h"

#include <iostream>

#include <sys/timeb.h>    // ftime(), struct timeb
#include <sys/time.h>
#include <assert.h>

using namespace std;

typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

Image ReadPGMFile(const char *filename);
Image ReadPGM(FILE *fp);
void WritePGM(const char* filename, Image image);
void SkipComments(FILE *fp);

inline u64 GetMicroTime()
{
    struct timeval t;
    gettimeofday(&t, NULL);
    return (u64)t.tv_sec*1000000+t.tv_usec;
}

Image ReadPGMFile(const char *filename)
{
    FILE *file;
    file = fopen (filename, "rb");
    if (! file) {
        cerr << "Could not open file: " << filename << endl;
        exit(1);
    }

    return ReadPGM(file);
}

Image ReadPGM(FILE *fp)
{
    int char1, char2, width, height, max, c1, c2, c3, r, c;
    Image image, nextimage;

    char1 = fgetc(fp);
    char2 = fgetc(fp);
    SkipComments(fp);
    c1 = fscanf(fp, "%d", &width);
    SkipComments(fp);
    c2 = fscanf(fp, "%d", &height);
    SkipComments(fp);
    c3 = fscanf(fp, "%d", &max);

    if (char1 != 'P' || char2 != '5' || c1 != 1 || c2 != 1 || c3 != 1 || max > 255) {
        cerr << "Input is not a standard raw 8-bit PGM file." << endl
             << "Use xv or pnmdepth to convert file to 8-bit PGM format." << endl;
        exit(1);
    }

    fgetc(fp);  // Discard exactly one byte after header.

    // Create floating point image with pixels in range [0,1].
    image = CreateImage(height, width);
    for (r = 0; r < height; r++)
        for (c = 0; c < width; c++)
            image->pixels[r*image->stride+c] = ((float) fgetc(fp)) / 255.0;

    //Check if there is another image in this file, as the latest PGM
    // standard allows for multiple images.
    SkipComments(fp);
    if (getc(fp) == 'P') {
        cerr << "ignoring other images" << endl;
        ungetc('P', fp);
        nextimage = ReadPGM(fp);
        //image->next = nextimage;
    }
    return image;
}

void WritePGM(const char* filename, Image image)
{
    int r, c, val;

    FILE* fp = fopen(filename,"wb");
    fprintf(fp, "P5\n%d %d\n255\n", image->cols, image->rows);

    int rows = image->rows, cols = image->cols, stride = image->stride;
    for (r = 0; r < rows; r++) {
        for (c = 0; c < cols; c++) {
            val = (int) (255.0 * image->pixels[r*stride+c]);
            if( val > 255 ) val = 255;
            else if( val < 0 ) val = 0;
            fputc(val, fp);
        }
    }

    fclose(fp);
}

void SkipComments(FILE *fp)
{
    int ch;

    fscanf(fp," "); // Skip white space.
    while ((ch = fgetc(fp)) == '#') {
        while ((ch = fgetc(fp)) != '\n'  &&  ch != EOF)
            ;
        fscanf(fp," ");
    }
    ungetc(ch, fp); // Replace last character read.
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
