#include "mycustom3ditem.h"
#include <QQuickWindow>
#include <QOpenGLContext>
#include <QSGSimpleTextureNode>
#include <QSGTexture>
#include <QSGMaterialShader>

// QRunnable 的实现，用于在 OpenGL 线程中安全删除 FBO
class CleanupJob : public QRunnable {
    QOpenGLFramebufferObject *fbo1, *fbo2;
public:
    CleanupJob(QOpenGLFramebufferObject *f1, QOpenGLFramebufferObject *f2) : fbo1(f1), fbo2(f2) {}
    void run() override { delete fbo1; delete fbo2; }
};

MyCustom3DItem::MyCustom3DItem(QQuickItem *parent) :
    QQuickItem(parent),
    m_projection(),
    m_view()
{
    setFlag(ItemHasContents, true);

    connect(this, &QQuickItem::windowChanged, this, [this](QQuickWindow *win) {
        if (win) {
            // 在主窗口的 OpenGL 上下文中连接渲染前信号
            connect(win, &QQuickWindow::beforeRendering, this, &MyCustom3DItem::onBeforeRendering, Qt::DirectConnection);
            m_timer.start(16, this); // 约 60 FPS
        } else {
            m_timer.stop();
        }
    });
}

MyCustom3DItem::~MyCustom3DItem()
{
    if (window()) {
        window()->scheduleRenderJob(new CleanupJob(m_fbo_msaa, m_fbo_resolved),
                                    QQuickWindow::BeforeSynchronizingStage);
        m_fbo_msaa = nullptr;
        m_fbo_resolved = nullptr;
    }
}

void MyCustom3DItem::updateScene()
{
    m_rotation += 1.0f;
    if (m_rotation >= 360.0f) m_rotation -= 360.0f;
    update(); // 标记 QML Item 需要重新渲染
}

void MyCustom3DItem::createMSAA_FBO(int width, int height)
{
    delete m_fbo_msaa;
    delete m_fbo_resolved;

    QOpenGLFramebufferObjectFormat format;
    format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
    format.setSamples(4); // !!! 关键：请求 4 倍 MSAA !!!
    m_fbo_msaa = new QOpenGLFramebufferObject(width, height, format);

    QOpenGLFramebufferObjectFormat formatResolved;
    formatResolved.setAttachment(QOpenGLFramebufferObject::NoAttachment);
    m_fbo_resolved = new QOpenGLFramebufferObject(width, height, formatResolved);
}

void MyCustom3DItem::onBeforeRendering()
{
    initializeOpenGLFunctions();

    int fboWidth = width() * window()->devicePixelRatio();
    int fboHeight = height() * window()->devicePixelRatio();
    if (!m_fbo_msaa || m_fbo_msaa->width() != fboWidth || m_fbo_msaa->height() != fboHeight) {
        createMSAA_FBO(fboWidth, fboHeight);
    }

    m_fbo_msaa->bind();
    glViewport(0, 0, fboWidth, fboHeight);
    glClearColor(0.18f, 0.18f, 0.18f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    render3DScene(); // 调用您的 3D 渲染代码

    glDisable(GL_DEPTH_TEST);

    // 将 MSAA FBO 解析到非 MSAA FBO
    QOpenGLFramebufferObject::blitFramebuffer(
        m_fbo_resolved, QRect(0, 0, fboWidth, fboHeight),
        m_fbo_msaa, QRect(0, 0, fboWidth, fboHeight),
        GL_COLOR_BUFFER_BIT, GL_NEAREST);

    // 返回 Qt Quick 的默认 FBO
    QOpenGLFramebufferObject::bindDefault();

    update(); // 标记 QML 场景需要更新（即调用 updatePaintNode）
}

void MyCustom3DItem::render3DScene()
{
    // --- 在此处实现您的 OBJ 模型绘制逻辑 ---
    // 设置 MVP 矩阵
    m_projection.setToIdentity();
    m_projection.perspective(45.0f, (float)m_fbo_msaa->width() / (float)m_fbo_msaa->height(), 0.1f, 100.0f);
    m_view.setToIdentity();
    m_view.translate(0.0f, 0.0f, -5.0f);
    m_view.rotate(m_rotation, 0.0f, 1.0f, 0.0f);

    // TODO: glUseProgram, glBindBuffer, glDrawArrays/Elements 等绘制代码
}


// ------------------------------------------------------------------
// updatePaintNode 的实现：使用 QSGSimpleTextureNode 将 FBO 纹理绘制到 QML 中
// ------------------------------------------------------------------
QSGNode *MyCustom3DItem::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)
{
    QSGSimpleTextureNode *node = static_cast<QSGSimpleTextureNode *>(oldNode);
    if (!node) {
        node = new QSGSimpleTextureNode();
        // node->setFiltering(QSGTexture::Linear);
    }

    if (m_rhi_texture_resolved && window()) {
        // !!! createTextureFromRhiTexture の使用 !!!
        QSGTexture *texture = window()->createTextureFromRhiTexture(m_rhi_texture_resolved);

        if (texture) {
            node->setTexture(texture);
            node->setRect(boundingRect());
            // RHI の座標系に合わせて反転が必要なら設定
            // node->setTextureCoordinates(QSGSimpleTextureNode::MirrorVertically);
        } else {
            node->setTexture(nullptr);
        }
    }
    return node;
}
