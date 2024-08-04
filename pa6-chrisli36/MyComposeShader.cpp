#include "MyComposeShader.h"

MyComposeShader::MyComposeShader(GShader* sh1, GShader* sh2) {
    this->sh1 = sh1;
    this->sh2 = sh2;
}

bool MyComposeShader::isOpaque() {
    return sh1->isOpaque() && sh2->isOpaque();
}

bool MyComposeShader::setContext(const GMatrix& ctm) {
    return sh1->setContext(ctm) && sh2->setContext(ctm);
}

void MyComposeShader::shadeRow(int x, int y, int count, GPixel row[]) {
    GPixel r1[count];
    GPixel r2[count];
    sh1->shadeRow(x, y, count, r1);
    sh2->shadeRow(x, y, count, r2);
    for(int i = 0; i < count; ++i) {
        row[i] = mul(r1[i], r2[i]);
    }
}
