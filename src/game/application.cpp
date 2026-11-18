#include "application.h"
#include "resources/textureatlas.h"

Application::Application()
{
}

void Application::initEngine()
{
    m_engine.initEngine(1280, 720, "LumosEngine test");
    m_engine.window()->setScrollCallback([this](double x, double y) {
            m_inputsystem.updateScrolling(x, y);
    });
    auto atlas = m_engine.resourceManager()->getResource<Lumos::TextureAtlas>("testsprite");
    auto cam = m_ecs.createEntity("cameraEntity");
    Lumos::Transform cameratransform;
    Lumos::Camera camera;
    cameratransform.position = {0.0f, 0.0f};
    cameratransform.size = {640, 320};
    camera.rotation = 1.0f;
    camera.zoom = 1.0f;
    m_ecs.addComponent(cam, cameratransform);
    m_ecs.addComponent(cam, camera);
    auto ent = m_ecs.createEntity("entity1");
    Lumos::Transform transform;
    Lumos::Sprite sprite;
    transform.position = {0.0f, 0.0f};
    transform.size = {10.0f, 10.0f};
    transform.rotation = 0.0f;
    sprite.color = {1.0f, 0.0f, 0.0f, 1.0f};
    sprite.uvrect = atlas->uv("floor2");
    m_ecs.addComponent(ent, transform);
    m_ecs.addComponent(ent, sprite);
}

void Application::run()
{
    m_engine.run();

    uint64_t framecount = 0;
    uint64_t lastfpsframe = 0;
    float timeaccumulator = 0.0f;
    float curfps = 0.0f;

    while(m_engine.running())
    {
        m_engine.processEvents();
        m_deltatime = m_engine.deltatime();
        framecount++;
        timeaccumulator += m_deltatime.count() * 0.001f;

        if (timeaccumulator >= 1.0f) {
            uint64_t framessincelastupdate = framecount - lastfpsframe;
            double avgms = 0.0;
            if (framessincelastupdate > 0 && timeaccumulator > 0.0f) {
                curfps = static_cast<float>(static_cast<double>(framessincelastupdate) / static_cast<double>(timeaccumulator));
                avgms = (timeaccumulator / static_cast<double>(framessincelastupdate)) * 1000.0;

            } else {

                curfps = std::max(curfps, 1.0f);
                avgms = static_cast<double>(m_deltatime.count());
            }
            std::string m_titleAndfps = std::format("Lumos engine test. Fps: {} | avg Ms: {}",
                    static_cast<int>(curfps), static_cast<int>(avgms));
            m_engine.setTitle(m_titleAndfps);
        }

        m_engine.update(m_deltatime);

        m_engine.render(m_ecs);
    }
    m_engine.waitIdle();
}

