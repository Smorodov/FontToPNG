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

// Получить изображение символа
FT_Glyph getGlyph(FT_Face face, uint32_t charcode);

// Получить кернинг между двумя символа
FT_Pos getKerning(FT_Face face, uint32_t leftCharcode, uint32_t rightCharcode);

// Сохранить картинку в PNG
void savePNG(uint8_t* image, int32_t width, int32_t height);

// Позиция и размер глифа в строке
struct Symbol
{
    // Позиция по горизонтали
    int32_t posX;
    // Позиция по вертикали (от базовой линии)
    int32_t posY;
    // Ширина глифа
    int32_t width;
    // Высота глифа
    int32_t height;

    FT_Glyph glyph;
};

FT_Glyph getGlyph(FT_Face face, uint32_t charcode)
{
    // Загрузка глифа в face->glyph с отрисовкой
    FT_Load_Char(face, charcode, FT_LOAD_RENDER);

    FT_Glyph glyph = 0;
    // Получаем глиф
    FT_Get_Glyph(face->glyph, &glyph);
    return glyph;
}


FT_Pos getKerning(FT_Face face, uint32_t leftCharcode, uint32_t rightCharcode)
{
    // Получаем индекс левого символа
    FT_UInt leftIndex = FT_Get_Char_Index(face, leftCharcode);
    // Получаем индекс правого символа
    FT_UInt rightIndex = FT_Get_Char_Index(face, rightCharcode);

    // Здесь будет хранится кернинг в формате 26.6
    FT_Vector delta;
    // Получаем кернинг для двух символов
    FT_Get_Kerning(face, leftIndex, rightIndex, FT_KERNING_DEFAULT, &delta);
    return delta.x;
}


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


void main(void)
{
    // Инициализация библиотеки
    FT_Library ftLibrary = 0;
    FT_Init_FreeType(&ftLibrary);

    // Загрузка шрифта arial.ttf из текущей папки
    FT_Face ftFace = 0;
    FT_New_Face(ftLibrary, "a_FuturaOrto.TTF", 0, &ftFace);
    // Установим размер символа для рендеринга
    FT_Set_Pixel_Sizes(ftFace, 100, 0);
    // Выводимая строка
    const std::wstring exampleString(L"!");

    // Набор готовых символов
    std::vector<Symbol> symbols;
    int32_t left = INT_MAX;
    int32_t top = INT_MAX;
    int32_t bottom = INT_MIN;
    uint32_t prevCharcode = 0;

    // Позиция текущего символа в формате 26.6
    int32_t posX = 0;

    for (size_t i = 0; i < exampleString.size(); ++i)
    {
        // Получаем код символа
        const uint32_t charcode = exampleString[i];

        // Получаем глиф для этого символа
        FT_Glyph glyph = getGlyph(ftFace, charcode);

        if (!glyph)
        {
            // Глифы в шрифте есть не для всех символов
            continue;
        }

        if (prevCharcode)
        {
            // Используем кернинг
            posX += getKerning(ftFace, prevCharcode, charcode);
        }
        prevCharcode = charcode;

        symbols.push_back(Symbol());
        Symbol& symb = symbols.back();

        FT_BitmapGlyph bitmapGlyph = (FT_BitmapGlyph)glyph;

        // Вычисляем горизонтальную позицию символа
        symb.posX = (posX >> 6) + bitmapGlyph->left;

        // Вычисляем вертикальную позицию символа относительно базовой 
        // линии. Отрицательные значения - сверху, положительные - снизу.
        symb.posY = -bitmapGlyph->top;

        // Ширина символа
        symb.width = bitmapGlyph->bitmap.width;
        // Высота символа
        symb.height = bitmapGlyph->bitmap.rows;

        // Ссылка на глиф
        symb.glyph = glyph;

        // Смещаем позицию текущего символа
        // (glyph->advance имеет формат 16.16, поэтому для приведения 
        // его к формату 26.6 необходимо сдвинуть число на 10 бит враво)
        posX += glyph->advance.x >> 10;

        // Вычисляем самую левую позицию
        left = std::min(left, symb.posX);

        // Вычисляем самую верхнюю позицию
        top = std::min(top, symb.posY);

        // Вычисляем самую нижнюю позицию
        bottom = std::max(bottom, symb.posY + symb.height);
    }

    for (std::size_t i = 0; i < symbols.size(); ++i)
    {
        // Смещаем все символы влево, чтобы строка примыкала к левой части
        symbols[i].posX -= left;
    }

    const Symbol& lastSymbol = symbols.back();

    // Ширина строки (изображения) - это крайняя правая 
    // точка последнего символа в строке
    const int32_t imageW = lastSymbol.posX + lastSymbol.width;

    // Высота строки (изображения)
    const int32_t imageH = bottom - top;
    // Выделяем память для картинки
    std::vector<uint8_t> image(imageW * imageH);

    for (std::size_t i = 0; i < symbols.size(); ++i)
    {
        const Symbol& symb = symbols[i];

        FT_BitmapGlyph bitmapGlyph = (FT_BitmapGlyph)symb.glyph;
        FT_Bitmap bitmap = bitmapGlyph->bitmap;

        for (int32_t srcY = 0; srcY < symb.height; ++srcY)
        {
            // Координата Y в итоговой картинке
            const int32_t dstY = symb.posY + srcY - top;

            for (int32_t srcX = 0; srcX < symb.width; ++srcX)
            {
                // Получаем пиксель из изображения символа,
                // (обязательно используйте pitch вместо width)
                const uint8_t c = bitmap.buffer[srcX + srcY * bitmap.pitch];

                // Если пиксель полностью прозрачный, то пропускаем его
                if (0 == c)
                {
                    continue;
                }

                // Приводим множество [0..255] к [0..1] для удобства блендинга
                const float a = c / 255.0f;

                // Координата X в итоговой картинке
                const int32_t dstX = symb.posX + srcX;

                // Вычислим смещение в итоговой картинке
                uint8_t* dst = image.data() + dstX + dstY * imageW;

                // Рисуем этот пиксель в итоговую картинку с блендингом
                dst[0] = uint8_t(a * 255 + (1 - a) * dst[0]);
            }
        }
    }

    savePNG(image.data(), imageW, imageH);

    // Освобождаем памяти для глифов
    for (std::size_t i = 0; i < symbols.size(); ++i)
    {
        FT_Done_Glyph(symbols[i].glyph);
    }

    // Освобождаем шрифт
    FT_Done_Face(ftFace);
    ftFace = 0;

    // Заканчиваем работу с библиотекой
    FT_Done_FreeType(ftLibrary);
    ftLibrary = 0;
}

