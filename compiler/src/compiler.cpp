#include "compiler.h"
#include "slz/slz.h"

FILE* data_file_h;
FILE* data_file_c;

vector<string> frames;
vector<bool> null_frames;

void compile_bitmap(string root_path, string bitmap_path, bool video)
{
    string bitmap_name = bitmap_path.substr(0, bitmap_path.size()-4);
    printf("compiling %s...\n", bitmap_name.c_str());

    FILE* bitmap = fopen((root_path+bitmap_path).c_str(), "rb");
    
    // bitmap header

    char bitmap_header[2];
    fread(bitmap_header, 1, 2, bitmap);
    unsigned int bitmap_size;
    fread(&bitmap_size, 4, 1, bitmap);
    unsigned int reserved;
    fread(&reserved, 4, 1, bitmap);
    unsigned int bitmap_offset;
    fread(&bitmap_offset, 4, 1, bitmap);
    
    // DIB header

    unsigned int dib_size;
    fread(&dib_size, 4, 1, bitmap);
    unsigned int bitmap_width;
    fread(&bitmap_width, 4, 1, bitmap);
    unsigned int bitmap_height;
    fread(&bitmap_height, 4, 1, bitmap);
    unsigned int reserved2;
    fread(&reserved2, 2, 1, bitmap);
    unsigned short bitmap_bpp;
    fread(&bitmap_bpp, 2, 1, bitmap);
    fread(&reserved2, 4, 1, bitmap);
    fread(&reserved2, 4, 1, bitmap);
    fread(&reserved2, 4, 1, bitmap);
    fread(&reserved2, 4, 1, bitmap);
    unsigned int bitmap_depth;
    fread(&bitmap_depth, 4, 1, bitmap);
    
    if (bitmap_depth==0)
        bitmap_depth = 16;


    bool incorrect_depth = false;
    if (bitmap_depth<3 || bitmap_depth>16 || bitmap_bpp!=4)
    {
        incorrect_depth = true;
        bitmap_depth = 16;
        bitmap_bpp = 4;

        if (!video)
        {
            printf("INCORRECT DEPTH\n");
            fclose(bitmap);
            return;
        }
    }

    int packed_width = (bitmap_width*bitmap_bpp)/16;
    int row_size = bitmap_width*bitmap_bpp/8;
    if (bitmap_width!=(packed_width*16)/bitmap_bpp)
    {
        printf("INCORRECT SIZE: %d,%d (%d,%f)\n", bitmap_width, (packed_width*16)/bitmap_bpp, bitmap_bpp, (bitmap_width*bitmap_bpp)/16.0f);
        fclose(bitmap);
        return;
    }
    
    bitmap_width = packed_width;

    if (video)
    {
        frames.push_back(bitmap_name);
    }
    else
    {
        fprintf(data_file_h, "    extern smeBitmap %s;\n", bitmap_name.c_str());
    }

    if (!incorrect_depth)
    {
        FILE* packed_input = fopen((root_path+"tmp.in").c_str(), "wb");
        {
            // palette

            fseek(bitmap, dib_size+14, SEEK_SET);
            
            for (int c=0 ; c<bitmap_depth ; ++c)
            {
                unsigned char b;
                fread(&b, 1, 1, bitmap);
                unsigned char g;
                fread(&g, 1, 1, bitmap);
                unsigned char r;
                fread(&r, 1, 1, bitmap);
                unsigned char a;
                fread(&a, 1, 1, bitmap);
            
                unsigned short v = ((r*15/255)<<8)+((g*15/255)<<12)+((b*15/255)<<0);
                fwrite(&v, 2, 1, packed_input);
            }

            // read image data

            unsigned short* image = (unsigned short*)malloc(bitmap_width*bitmap_height*2);

            for (int y=0 ; y<bitmap_height ; ++y)
            {
                fseek(bitmap, bitmap_offset+(bitmap_height-y-1)*row_size, SEEK_SET);
                for (int x=0 ; x<bitmap_width ; ++x)
                {
                    fread(image+y*bitmap_width+x, 2, 1, bitmap);
                    ((unsigned char*)(image+y*bitmap_width+x))[0] += 1+(1<<4);
                    ((unsigned char*)(image+y*bitmap_width+x))[1] += 1+(1<<4);
                }
            }

            fwrite(image, 2, bitmap_width*bitmap_height, packed_input);
            free(image);
        }
        fclose(packed_input);
        
        // compress

        packed_input = fopen((root_path+"tmp.in").c_str(), "rb");
        FILE* packed_output = fopen((root_path+"tmp.out").c_str(), "wb");
        compress(packed_input, packed_output, FORMAT_SLZ16);
        fclose(packed_output);
        fclose(packed_input);
        
        packed_output = fopen((root_path+"tmp.out").c_str(), "rb");
        fseek(packed_output, 0, SEEK_END);
        long size = ftell(packed_output);
        unsigned char* packed = (unsigned char*)malloc(size);
        fseek(packed_output, 0, SEEK_SET);
        fread(packed, 1, size, packed_output);
        fclose(packed_output);
        
        // write

        if (video)
        {
            fprintf(data_file_c, "const u8 %s[] = {", bitmap_name.c_str());
        }
        else
        {
            fprintf(data_file_c, "const u8 %s_packed[] = {", bitmap_name.c_str());
        }
        
        for (int i=0 ; i<size ; ++i)
        {
            fprintf(data_file_c, "%u", packed[i]);
            if (i<size-1)
                fprintf(data_file_c, ",");
        }
        free(packed);

        fprintf(data_file_c, "};\n");

        null_frames.push_back(false);
    }
    else
    {
        null_frames.push_back(true);
    }

    if (!video)
    {   
        fprintf(data_file_c, "smeBitmap %s = {\n", bitmap_name.c_str());
        fprintf(data_file_c, "    %i,\n", bitmap_width);
        fprintf(data_file_c, "    %i,\n", bitmap_height);
        fprintf(data_file_c, "    %i,\n", bitmap_depth);
        fprintf(data_file_c, "    NULL,\n");
        fprintf(data_file_c, "    NULL,\n");
        fprintf(data_file_c, "    NULL,\n");
        fprintf(data_file_c, "    %s_packed\n", bitmap_name.c_str());
        fprintf(data_file_c, "};\n");
    }

    fclose(bitmap);
}

void compile_map(string root_path, string map_path)
{
    string map_name = map_path.substr(0, map_path.size()-4);
    printf("compiling map %s...\n", map_name.c_str());

    fprintf(data_file_h, "    extern const u16 %s[];\n", map_name.c_str());
    fprintf(data_file_c, "const u16 %s[] = {", map_name.c_str());
    
    FILE* map = fopen((root_path+map_path).c_str(), "rt");
    int tile_index;
    while (feof(map)==0 && fscanf(map, "%d", &tile_index))
    {
        tile_index = tile_index==-1?0:tile_index+0x10;
        fprintf(data_file_c, "%d,\n", tile_index);
        fgetc(map);
    }
    fclose(map);
    fprintf(data_file_c, "0};\n\n");
}

int main(int argc, char* argv[])
{
    string root_path = argv[1];
    printf("compiling data...\n");

    data_file_h = fopen((root_path+"data.h").c_str(), "wt");
    fprintf(data_file_h, "#ifndef __DATA_H__\n");
    fprintf(data_file_h, "#define __DATA_H__\n\n");
    fprintf(data_file_h, "    #include \"sme_bitmap.h\"\n\n");

    data_file_c = fopen((root_path+"data.c").c_str(), "wt");
    fprintf(data_file_c, "#include \"data.h\"\n\n");

    WIN32_FIND_DATA find_data;
    HANDLE handle = FindFirstFile((root_path+"*.bmp").c_str(), &find_data);
    if (handle!=INVALID_HANDLE_VALUE)
    {
        do
        {
            if (!(find_data.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
                compile_bitmap(root_path, find_data.cFileName, false);
        }
        while (FindNextFile(handle, &find_data)!=0);
    }
    FindClose(handle);

    // Video

    handle = FindFirstFile((root_path+"video/*.bmp").c_str(), &find_data);
    if (handle!=INVALID_HANDLE_VALUE)
    {
        fprintf(data_file_h, "    extern const int FrameCount;\n");
        fprintf(data_file_h, "    extern const u8* Frames[];\n");    
    
        do
        {
            if (!(find_data.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
                compile_bitmap(root_path+"video/", find_data.cFileName, true);
        }
        while (FindNextFile(handle, &find_data)!=0);
   
        fprintf(data_file_c, "const u8* Frames[] = {\n");
        for (int i=0 ; i<frames.size() ; ++i)
        {
            if (i>0)
                fprintf(data_file_c, ",\n");    

            if (null_frames[i]) fprintf(data_file_c, "NULL");
            else fprintf(data_file_c, "%s", frames[i].c_str());
        }
        fprintf(data_file_c, "\n};\n");
        
        fprintf(data_file_c, "const int FrameCount = %d;\n", frames.size());
        fclose(data_file_c);
    }
    FindClose(handle);

    // Maps

    handle = FindFirstFile((root_path+"*.csv").c_str(), &find_data);
    if (handle!=INVALID_HANDLE_VALUE)
    {
        do
        {
            if (!(find_data.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
                compile_map(root_path, find_data.cFileName);
        }
        while (FindNextFile(handle, &find_data)!=0);
    }
    FindClose(handle);

    fprintf(data_file_h, "\n#endif\n");
    fclose(data_file_h);
    return 0;
}

