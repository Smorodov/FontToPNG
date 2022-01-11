// ���������� FreeType
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

// ���������� libpng (http://libpng.sourceforge.net/index.html)
#include <png.h>

// ��������� uint8_t, int32_t � �.�.
#include <stdint.h>
// ��������� std::string
#include <string>
// ��������� std::vector
#include <vector>

// �������� ����������� �������
FT_Glyph getGlyph(FT_Face face, uint32_t charcode);

// �������� ������� ����� ����� �������
FT_Pos getKerning(FT_Face face, uint32_t leftCharcode, uint32_t rightCharcode);

// ��������� �������� � PNG
void savePNG(uint8_t* image, int32_t width, int32_t height);

// ������� � ������ ����� � ������
struct Symbol
{
    // ������� �� �����������
    int32_t posX;
    // ������� �� ��������� (�� ������� �����)
    int32_t posY;
    // ������ �����
    int32_t width;
    // ������ �����
    int32_t height;

    FT_Glyph glyph;
};

FT_Glyph getGlyph(FT_Face face, uint32_t charcode)
{
    // �������� ����� � face->glyph � ����������
    FT_Load_Char(face, charcode, FT_LOAD_RENDER);

    FT_Glyph glyph = 0;
    // �������� ����
    FT_Get_Glyph(face->glyph, &glyph);
    return glyph;
}


FT_Pos getKerning(FT_Face face, uint32_t leftCharcode, uint32_t rightCharcode)
{
    // �������� ������ ������ �������
    FT_UInt leftIndex = FT_Get_Char_Index(face, leftCharcode);
    // �������� ������ ������� �������
    FT_UInt rightIndex = FT_Get_Char_Index(face, rightCharcode);

    // ����� ����� �������� ������� � ������� 26.6
    FT_Vector delta;
    // �������� ������� ��� ���� ��������
    FT_Get_Kerning(face, leftIndex, rightIndex, FT_KERNING_DEFAULT, &delta);
    return delta.x;
}


void savePNG(uint8_t* image, int32_t width, int32_t height)
{
    // ���� ��� ���������� ��������
    FILE* f = fopen("output.png", "wb");

    png_structp png_ptr =
        png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);

    png_infop info_ptr = png_create_info_struct(png_ptr);

    png_init_io(png_ptr, f);

    // ����������� � ������� RGBA �� 8 ��� �� 
    // ����� � �� ������ ������ �� �������
    png_set_IHDR(
        png_ptr,
        info_ptr,
        width,
        height,
        8,
        PNG_COLOR_TYPE_RGBA,
        PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_BASE,
        PNG_FILTER_TYPE_BASE);

    png_write_info(png_ptr, info_ptr);

    // ���� ������ � ������� RGBA, 4 ������
    std::vector<uint8_t> row(width * 4);

    // ��������� PNG ���������
    for (int32_t y = 0; y < height; ++y)
    {
        // ����������� ���� ������ �� ������������� � ������ RGBA
        for (int32_t x = 0; x < width; ++x)
        {
            // ���� ���������� ��� ���� �������� 0x202020
            unsigned char col = image[y * width + x];
            row[x * 4 + 0] = 0;
            row[x * 4 + 1] = col;
            row[x * 4 + 2] = 255;
            // ������������ ���� �� �������� ������
            row[x * 4 + 3] = 255;//col;
        }

        // ��������� ������ � PNG
        png_write_row(png_ptr, row.data());
    }

    png_write_end(png_ptr, 0);

    // ��������� ������, ����������� �������
    fclose(f);
    png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
    png_destroy_write_struct(&png_ptr, 0);
}


void main(void)
{
    // ������������� ����������
    FT_Library ftLibrary = 0;
    FT_Init_FreeType(&ftLibrary);

    // �������� ������ arial.ttf �� ������� �����
    FT_Face ftFace = 0;
    FT_New_Face(ftLibrary, "a_FuturaOrto.TTF", 0, &ftFace);
    // ��������� ������ ������� ��� ����������
    FT_Set_Pixel_Sizes(ftFace, 100, 0);
    // ��������� ������
    const std::wstring exampleString(L"!");

    // ����� ������� ��������
    std::vector<Symbol> symbols;
    int32_t left = INT_MAX;
    int32_t top = INT_MAX;
    int32_t bottom = INT_MIN;
    uint32_t prevCharcode = 0;

    // ������� �������� ������� � ������� 26.6
    int32_t posX = 0;

    for (size_t i = 0; i < exampleString.size(); ++i)
    {
        // �������� ��� �������
        const uint32_t charcode = exampleString[i];

        // �������� ���� ��� ����� �������
        FT_Glyph glyph = getGlyph(ftFace, charcode);

        if (!glyph)
        {
            // ����� � ������ ���� �� ��� ���� ��������
            continue;
        }

        if (prevCharcode)
        {
            // ���������� �������
            posX += getKerning(ftFace, prevCharcode, charcode);
        }
        prevCharcode = charcode;

        symbols.push_back(Symbol());
        Symbol& symb = symbols.back();

        FT_BitmapGlyph bitmapGlyph = (FT_BitmapGlyph)glyph;

        // ��������� �������������� ������� �������
        symb.posX = (posX >> 6) + bitmapGlyph->left;

        // ��������� ������������ ������� ������� ������������ ������� 
        // �����. ������������� �������� - ������, ������������� - �����.
        symb.posY = -bitmapGlyph->top;

        // ������ �������
        symb.width = bitmapGlyph->bitmap.width;
        // ������ �������
        symb.height = bitmapGlyph->bitmap.rows;

        // ������ �� ����
        symb.glyph = glyph;

        // ������� ������� �������� �������
        // (glyph->advance ����� ������ 16.16, ������� ��� ���������� 
        // ��� � ������� 26.6 ���������� �������� ����� �� 10 ��� �����)
        posX += glyph->advance.x >> 10;

        // ��������� ����� ����� �������
        left = std::min(left, symb.posX);

        // ��������� ����� ������� �������
        top = std::min(top, symb.posY);

        // ��������� ����� ������ �������
        bottom = std::max(bottom, symb.posY + symb.height);
    }

    for (std::size_t i = 0; i < symbols.size(); ++i)
    {
        // ������� ��� ������� �����, ����� ������ ��������� � ����� �����
        symbols[i].posX -= left;
    }

    const Symbol& lastSymbol = symbols.back();

    // ������ ������ (�����������) - ��� ������� ������ 
    // ����� ���������� ������� � ������
    const int32_t imageW = lastSymbol.posX + lastSymbol.width;

    // ������ ������ (�����������)
    const int32_t imageH = bottom - top;
    // �������� ������ ��� ��������
    std::vector<uint8_t> image(imageW * imageH);

    for (std::size_t i = 0; i < symbols.size(); ++i)
    {
        const Symbol& symb = symbols[i];

        FT_BitmapGlyph bitmapGlyph = (FT_BitmapGlyph)symb.glyph;
        FT_Bitmap bitmap = bitmapGlyph->bitmap;

        for (int32_t srcY = 0; srcY < symb.height; ++srcY)
        {
            // ���������� Y � �������� ��������
            const int32_t dstY = symb.posY + srcY - top;

            for (int32_t srcX = 0; srcX < symb.width; ++srcX)
            {
                // �������� ������� �� ����������� �������,
                // (����������� ����������� pitch ������ width)
                const uint8_t c = bitmap.buffer[srcX + srcY * bitmap.pitch];

                // ���� ������� ��������� ����������, �� ���������� ���
                if (0 == c)
                {
                    continue;
                }

                // �������� ��������� [0..255] � [0..1] ��� �������� ���������
                const float a = c / 255.0f;

                // ���������� X � �������� ��������
                const int32_t dstX = symb.posX + srcX;

                // �������� �������� � �������� ��������
                uint8_t* dst = image.data() + dstX + dstY * imageW;

                // ������ ���� ������� � �������� �������� � ����������
                dst[0] = uint8_t(a * 255 + (1 - a) * dst[0]);
            }
        }
    }

    savePNG(image.data(), imageW, imageH);

    // ����������� ������ ��� ������
    for (std::size_t i = 0; i < symbols.size(); ++i)
    {
        FT_Done_Glyph(symbols[i].glyph);
    }

    // ����������� �����
    FT_Done_Face(ftFace);
    ftFace = 0;

    // ����������� ������ � �����������
    FT_Done_FreeType(ftLibrary);
    ftLibrary = 0;
}

