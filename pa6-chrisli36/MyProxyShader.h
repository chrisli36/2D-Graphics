#ifndef MYPROXYSHADER_H
#define MYPROXYSHADER_H

#include "include/GShader.h"
#include "include/GMatrix.h"

class MyProxyShader : public GShader {
public:
    MyProxyShader(GShader* s, const GMatrix& extra) : shader(s), extraTransform(extra) {};
    bool isOpaque() override { return shader->isOpaque(); };
    virtual bool setContext(const GMatrix& ctm) override { return shader->setContext(ctm * extraTransform); };
    virtual void shadeRow(int x, int y, int count, GPixel row[]) override { shader->shadeRow(x, y, count, row); };

private:
    GShader* shader;
    const GMatrix extraTransform;
};

#endif