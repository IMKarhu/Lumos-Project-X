#include "application.h"
#include "resources/textureatlas.h"

Application::Application()
{
}

void Application::initEngine()
{
    m_engine.initEngine(1280, 720, "LumosEngine test");
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
    // m_ecs.addComponent(ent, transform);
    // RenderData renderdata{};
    // renderdata.cameradata.position = {0.0f, 0.0f};
    // renderdata.cameradata.size = {640, 320};
    // RenderObject obj;
    // obj.position = {0.0f, 0.0f};
    // obj.size = {10.0f, 10.0f};
    // obj.rotation = 0.0f;
    // obj.color = {1.0f, 0.0f, 0.0f, 1.0f};
    // obj.uvrect = atlas->uv("floor2");
    // renderdata.renderobjects.push_back(obj);
    // RenderObject obj2;
    // obj2.position = {200.0f, 0.0f};
    // obj2.size = {100.0f, 100.0f};
    // obj2.rotation = 45.0f;
    // obj2.color = {1.0f, 1.0f, 0.0f, 1.0f};
    // obj2.uvrect = atlas->uv("floor");
    // renderdata.renderobjects.push_back(obj2);
    // RenderObject obj3;
    // obj3.position = {0.0f, 100.0f};
    // obj3.size = {100.0f, 100.0f};
    // obj3.rotation = 90.0f;
    // obj3.color = {0.5f, 1.0f, 1.0f, 1.0f};
    // obj3.uvrect = {0.0f, 0.0f, 1.0f, 1.0f};
    // renderdata.renderobjects.push_back(obj3);
    // RenderObject obj4;
    // obj4.position = {-200.0f, 0.0f};
    // obj4.size = {100.0f, 100.0f};
    // obj4.rotation = 90.0f;
    // obj4.color = {0.0f, 0.0f, 1.0f, 1.0f};
    // obj4.uvrect = {0.0f, 0.0f, 1.0f, 1.0f};
    // renderdata.renderobjects.push_back(obj4);
    // RenderObject obj5;
    // obj5.position = {0.0f, -100.0f};
    // obj5.size = {100.0f, 100.0f};
    // obj5.rotation = 0.0f;
    // obj5.color = {0.0f, 1.0f, 0.0f, 1.0f};
    // obj5.uvrect = {0.0f, 0.0f, 1.0f, 1.0f};
    // renderdata.renderobjects.push_back(obj5);

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
            // std::string m_titleAndfps = std::format("Lumos engine test. Fps: {:.1f}", curfps);
            std::string m_titleAndfps = std::format("Lumos engine test. Fps: {} | avg Ms: {}",
                    static_cast<int>(curfps), static_cast<int>(avgms));
            m_engine.setTitle(m_titleAndfps);
        }

        m_engine.update(m_deltatime);

        m_engine.render(m_ecs);
    }
    m_engine.waitIdle();
}

