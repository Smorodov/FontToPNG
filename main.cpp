#include <iostream>
// Подключаем FreeType
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
// Подключаем libpng (http://libpng.sourceforge.net/index.html)
#include <png.h>
// Поддержка uint8_t, int32_t и т.д.
#include <stdint.h>
// Поддержка std::string
#include <string>
// Поддержка std::vector
#include <vector>
#include <icons_font_awesome_4.h>
// Сохранить картинку в PNG
void savePNG(uint8_t* image, int32_t width, int32_t height)
{
    // Файл для сохранения картинки
    FILE* f = fopen("output.png", "wb");

    png_structp png_ptr =
        png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);

    png_infop info_ptr = png_create_info_struct(png_ptr);

    png_init_io(png_ptr, f);

    // Изображение в формате RGBA по 8 бит на 
    // канал и по четыре канала на пиксель
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

    // Одна строка в формате RGBA, 4 канала
    std::vector<uint8_t> row(width * 4);

    // Сохраняем PNG построчно
    for (int32_t y = 0; y < height; ++y)
    {
        // Преобразуем нашу строку из одноканальной в формат RGBA
        for (int32_t x = 0; x < width; ++x)
        {
            // Цвет одинаковый для всех пикселей 0x202020
            unsigned char col = image[y * width + x];
            row[x * 4 + 0] = 0;
            row[x * 4 + 1] = col;
            row[x * 4 + 2] = 255;
            // Прозрачность берём из исходных данных
            row[x * 4 + 3] = 255;//col;
        }

        // Сохраняем строку в PNG
        png_write_row(png_ptr, row.data());
    }

    png_write_end(png_ptr, 0);

    // Закончили работу, освобождаем ресурсы
    fclose(f);
    png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
    png_destroy_write_struct(&png_ptr, 0);
}

FT_Glyph getGlyph(FT_Face face, uint32_t charcode)
{
    // Загрузка глифа в face->glyph с отрисовкой
    FT_Load_Char(face, charcode, FT_LOAD_RENDER);

    FT_Glyph glyph = 0;
    // Получаем глиф
    FT_Get_Glyph(face->glyph, &glyph);
    return glyph;
}



bool GenerateGlyph(FT_Face& face,
    FT_Glyph& glyph,
    unsigned int fontSize,
    unsigned int ImgSize,
    int maxDescent,
    unsigned char*& glyphImageData,
    unsigned int& w,
    unsigned int& h)
{
    FT_Glyph_To_Bitmap(&glyph, ft_render_mode_normal, 0, 1);
    FT_BitmapGlyph bitmap_glyph = (FT_BitmapGlyph)glyph;
    FT_Bitmap& bitmap = bitmap_glyph->bitmap;

    glyphImageData = new unsigned char[ImgSize * ImgSize];
    memset(glyphImageData, 0, ImgSize * ImgSize);

    // get the glyph metrics
    const FT_Glyph_Metrics& glyphMetrics = face->glyph->metrics;
    // find the character that reaches below the baseline by the biggest value
    int Descent = (face->glyph->metrics.height >> 6) - face->glyph->bitmap_top;
    
    
    for (int j = 0; j < ImgSize; j++)
    {
        for (int i = 0; i < ImgSize; i++)
        {
            int ind1 = (ImgSize * j + i);
            if (j < 0 || j >= ImgSize)
            {
                break;
            }

            if ((j == ImgSize - maxDescent)||( j== ImgSize - maxDescent-fontSize))
            {
                glyphImageData[ind1] = 255;
            }

        }
    }

    
    int offX = 0.5 * (ImgSize - bitmap.width);
    int deltaY = maxDescent - Descent;
    int offY = -deltaY;

    for (int j = 0; j < bitmap.rows; j++)
    {
        for (int i = 0; i < bitmap.width; i++)
        {
            int xx = (i + offX);
            int yy = ImgSize - j + offY;
                        
            if (xx<0 || xx>=ImgSize)
            {
                continue;
            }
            if (yy<0 || yy>=ImgSize)
            {
                continue;
            }
            int ind1 = (ImgSize * yy + xx);
            int ind2 = (bitmap.rows - 1 - j) * bitmap.pitch + i;
            glyphImageData[ind1] = bitmap.buffer[ind2];            
        }
    }
    w = ImgSize;
    h = ImgSize;
    return true;
}

void main(void)
{
    // Инициализация библиотеки
    FT_Library ftLibrary = 0;
    FT_Init_FreeType(&ftLibrary);
    // Загрузка шрифта arial.ttf из текущей папки
    FT_Face ftFace = 0;
    FT_New_Face(ftLibrary, "fontawesome-webfont.ttf", 0, &ftFace);
    // Установим размер символа для рендеринга
    int fontH = 100;
    FT_Set_Pixel_Sizes(ftFace, fontH, 0);

    int maxAscent = int(ftFace->ascender * (ftFace->size->metrics.y_scale / 65536.0)) >> 6;
    // ---------------------------------------
    // ---------------------------------------// 
    // create an array to save the character widths    
    // we need to find the character that goes below the baseline by the biggest value    
    int maxDescent = 0;
    int imageHeight = 0;
    uint32_t charcodeM = 0;
    for (unsigned int i = 0; i < 0xFFFF; ++i)
    {
        // get the glyph index from character code
        unsigned int glyphIndex = i;// FT_Get_Char_Index(ftFace, i);
        // load the glyph image into the slot
        unsigned int error = FT_Load_Glyph(ftFace, glyphIndex, FT_LOAD_DEFAULT);
        if (error)
        {
            //std::cout << "BitmapFontGenerator > failed to load glyph, error code: " << error << std::endl;
            continue;
        }
        // get the glyph metrics
        const FT_Glyph_Metrics& glyphMetrics = ftFace->glyph->metrics;
        
        if ( ((glyphMetrics.height >> 6) - ftFace->glyph->bitmap_top+ maxDescent) > (int)maxDescent)
        {
        maxDescent = (ftFace->glyph->metrics.height/64.0) - ftFace->glyph->bitmap_top;
        charcodeM = glyphIndex;
        }
        imageHeight = maxAscent + maxDescent;
    }
    // maxAscent = int(face->ascender * (face->size->metrics.y_scale / 65536.0)) >> 6;
    // if ((glyph->metrics.height >> 6) - glyph->bitmap_top > (int)maxDescent) 
    // {
    // maxDescent = (glyph->metrics.height >> 6) - glyph->bitmap_top;
    // }
    // imageHeight = maxAscent + maxDescent
    // https://stackoverflow.com/questions/62374506/how-do-i-align-glyphs-along-the-baseline-with-freetype
    // ---------------------------------------
    // https://levelup.gitconnected.com/how-to-create-a-bitmap-font-with-freetype-58e8c31878a9
    // ---------------------------------------

    // Код символа
    uint32_t charcode = 0xf2b9;// /*L'Ц'; // */ charcodeM;//uint32_t(L'Й');
    // get the glyph index from character code
    //unsigned int glyphIndex = FT_Get_Char_Index(ftFace, charcode);
    FT_Glyph glyph;
    glyph = getGlyph(ftFace, charcode);
    if (!glyph)
    {
        // Глифы в шрифте есть не для всех символов
        return;
    }
    unsigned char* glyphImageData = 0;
    unsigned int W = 0, H = 0;
    GenerateGlyph(ftFace, glyph, fontH, imageHeight, maxDescent, glyphImageData, W, H);
    savePNG(glyphImageData, W, H);
    delete[]glyphImageData;
    // 
    // Освобождаем памяти для глифов
    FT_Done_Glyph(glyph);
    // Освобождаем шрифт
    FT_Done_Face(ftFace);
    ftFace = 0;
    // Заканчиваем работу с библиотекой
    FT_Done_FreeType(ftLibrary);
    ftLibrary = 0;
}

