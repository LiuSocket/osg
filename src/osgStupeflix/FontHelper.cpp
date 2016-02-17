#include <osgStupeflix/FontHelper>

typedef std::pair< unsigned int, unsigned int > FontResolution;

bool osgStupeflix::isCharInFont(unsigned int charcode, osgText::Font* font) {
    osgText::Glyph* g = font->getGlyph(FontResolution(1,1), charcode);
    return (g != 0);
}

