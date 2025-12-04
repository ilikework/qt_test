#ifndef MYCUSTOM3DITEM_H
#define MYCUSTOM3DITEM_H

#include <QQuickItem>
#include <QOpenGLFunctions>
#include <QOpenGLFramebufferObject>
#include <QMatrix4x4>
#include <QBasicTimer>
#include <QSGNode>
#include <QSGGeometryNode>
#include <QSGGeometry>
#include <QSGMaterial>
#include <QRunnable>
#include <QSGTexture>
//#include <QSGVertex>

class MyCustom3DItem : public QQuickItem, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    MyCustom3DItem(QQuickItem *parent = nullptr);
    ~MyCustom3DItem();

protected:
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *) override;

private slots:
    void onBeforeRendering();
    void updateScene();

private:
    void render3DScene(); // 实际的 OpenGL 渲染代码
    void createMSAA_FBO(int width, int height); // 创建抗锯齿 FBO

    QOpenGLFramebufferObject *m_fbo_msaa = nullptr;
    QOpenGLFramebufferObject *m_fbo_resolved = nullptr;
    QRhiTexture *m_rhi_texture_resolved = nullptr;

    QBasicTimer m_timer;
    float m_rotation = 0.0f;
    QMatrix4x4 m_projection;
    QMatrix4x4 m_view;
};

#endif // MYCUSTOM3DITEM_H
