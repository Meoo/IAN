
/*#include <SFML/Graphics.hpp>

#include <world/World.hpp>
#include <Events.hpp>

#include <Trace.hpp>*/

//#include "js/RequiredDefines.h"
#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>
#pragma warning(push)
#pragma warning(disable:4127 4251 4100 4800)
#include "jsapi.h"
#pragma warning(pop)

using namespace JS;

// The class of the global object.
static JSClass globalClass = {
    "global",
    JSCLASS_GLOBAL_FLAGS
};

// The error reporter callback.
void reportError(JSContext *cx, const char *message, JSErrorReport *report) {
     fprintf(stderr, "%s:%u:%s\n",
             report->filename ? report->filename : "[no filename]",
             (unsigned int) report->lineno,
             message);
}

int run(JSContext *cx) {
    // Enter a request before running anything in the context.
    JSAutoRequest ar(cx);

    // Create the global object and a new compartment.
    RootedObject global(cx);
    global = JS_NewGlobalObject(cx, &globalClass, nullptr,
                                JS::DontFireOnNewGlobalHook);
    if (!global)
        return 1;

    // Enter the new global object's compartment.
    JSAutoCompartment ac(cx, global);

    // Populate the global object with the standard globals, like Object and
    // Array.
    if (!JS_InitStandardClasses(cx, global))
        return 1;

    // Your application code here. This may include JSAPI calls to create your
    // own custom JS objects and run scripts.

    return 0;
}

int mainz(int argc, const char *argv[]) {
    // Initialize the JS engine.
    if (!JS_Init())
       return 1;

    // Create a JS runtime.
    JSRuntime *rt = JS_NewRuntime(8L * 1024L * 1024L);
    if (!rt)
       return 1;

    // Create a context.
    JSContext *cx = JS_NewContext(rt, 8192);
    if (!cx)
       return 1;
    JS_SetErrorReporter(rt, reportError);

    int status = run(cx);

    // Shut everything down.
    JS_DestroyContext(cx);
    JS_DestroyRuntime(rt);
    JS_ShutDown();

    return status;
}

/*namespace
{
    const int W = 256;
    const int H = 256;

    const int smoothradius = 1;
    const float smooth[3][3] = 
    {
        {1,   2,   1},
        {2,   0,   2},
        {1,   2,   1},
    };

    bool toggle = true;
}

struct TileData
{
    float temperature = 0;
};

class Tile
{
public:
    bool wall;


    TileData a, b;

    TileData & getWrite() { return toggle ? a : b; };
    const TileData & getRead() const { return toggle ? b : a; };


    sf::Color getColor() const
    {
        if (wall)
            return sf::Color::Black;

        unsigned val = getRead().temperature;

        return sf::Color(255, 255 - val, 255 - val);
    }
};

bool produce = true;
bool consume = true;

class World
{
public:
    Tile data[W][H];

    Tile & at(unsigned x, unsigned y) { return data[x % W][y % H]; }
    const Tile & at(unsigned x, unsigned y) const { return data[x % W][y % H]; }

    void update()
    {
        for (unsigned i = 0; i < W*H; ++i)
        {
            unsigned x = i % W;
            unsigned y = i / W;

            Tile & tile = at(x, y);
            TileData & data = tile.getWrite();
            if (tile.wall) continue;

            float acc = 0;
            float fact = 0;
            for (int dx = - smoothradius; dx <= smoothradius; ++dx)
            for (int dy = - smoothradius; dy <= smoothradius; ++dy)
            {
                const Tile & rtile = at(x + dx, y + dy);
                const TileData & rdata = rtile.getRead();
                if (!rtile.wall)
                {
                    float smoothy = smooth[dx + smoothradius][dy + smoothradius];
                    if (smoothy == 0) continue;
                    float tfact = (std::fabs(data.temperature - rdata.temperature) + 500) * smoothy;
                    acc += rdata.temperature * tfact;
                    fact += tfact;
                }
            }

            if (fact > 0)
                data.temperature = (acc / fact);
        }

        if (produce)
        {
            at(10, 10).getWrite().temperature = 255;
            at(50, 50).getWrite().temperature = 255;
            at(53, 53).getWrite().temperature = 255;
            at(184, 104).getWrite().temperature = 255;
            at(150, 250).getWrite().temperature = 255;
            at(110, 30).getWrite().temperature = 255;
        }

        if (consume)
        {
            at(25, 25).getWrite().temperature = 0;
            at(144, 101).getWrite().temperature = 0;
            at(144, 104).getWrite().temperature = 0;
            at(144, 107).getWrite().temperature = 0;
            at(130, 30).getWrite().temperature = 0;
        }

        toggle = !toggle;
    }
};

World world;

void quad(World & world, unsigned x, unsigned y, unsigned w, unsigned h)
{
    // Top & bottom
    for (unsigned a = x; a < x+w; ++a)
    {
        world.at(a, y).wall = (x + a) % 172 == 13 ? false : true;
        world.at(a, y + h).wall = (a + h) % 130 == 13 ? false : true;
    }

    // Left & right
    for (unsigned b = y; b < y+h; ++b)
    {
        world.at(x, b).wall = (x + b) % 120 == 13 ? false : true;
        world.at(x + w, b).wall = (b - w) % 141 == 13 ? false : true;
    }
}

int main(int, char **)
{
    /*M_LOG_FIXME("Fixme");
    M_LOG_DEBUG("Debug");
    M_LOG_INFO("Info");
    M_LOG_WARN("Warn");
    M_LOG_ERROR("Error");
    M_LOG_CRIT("Crit");

    M_LOG_INFO("Built on ", __DATE__, " at ", __TIME__, " by ", __USER__);* /
    
    M_LOG_INFO("World");

    quad(world, 110, 210, 80, 60);
    quad(world, 40, 20, 150, 90);
    quad(world, 140, 30, 70, 370);
    quad(world, 60, 170, 100, 30);
    quad(world, 210, 150, 40, 90);
    quad(world, 100, 100, 200, 200);

    M_LOG_INFO("Points");
    sf::VertexArray points(sf::Points, W*H);
    for (unsigned i = 0; i < W*H; ++i)
    {
        sf::Vertex & vert = points[i];
        vert.position.x = i % W;
        vert.position.y = i / W;
    }

    M_LOG_INFO("Window");
    sf::RenderWindow win;
    win.create(sf::VideoMode(W, H), "IAN Sim",
            sf::Style::Titlebar | sf::Style::Close);

    win.setFramerateLimit(30);
    //win.setVerticalSyncEnabled(true);
    win.setKeyRepeatEnabled(false);

    int frame = 0;
    auto begin = std::chrono::steady_clock::now();
    bool sim = false, simonce = false;

    M_LOG_INFO("Main loop");
    while(win.isOpen())
    {
        //M_PROFILER_SCOPE(main_loop);

        {
            //M_PROFILER_SCOPE(events);

            sf::Event event;
            while (win.pollEvent(event))
            {
                switch(event.type)
                {
                case sf::Event::Closed:
                    win.close();
                    break;

                case sf::Event::KeyPressed:
                    if (event.key.code == sf::Keyboard::Escape) win.close();
                    if (event.key.code == sf::Keyboard::Space) sim = !sim;
                    if (event.key.code == sf::Keyboard::B) simonce = true;
                    if (event.key.code == sf::Keyboard::J) produce = !produce;
                    if (event.key.code == sf::Keyboard::K) consume = !consume;
                    break;

                case sf::Event::MouseButtonPressed:
                    M_LOG_INFO("Temp ", event.mouseButton.x, "x", event.mouseButton.y, " : ", world.at(event.mouseButton.x, event.mouseButton.y).getRead().temperature);
                    break;

                default:
                    break;
                }
            }
        }


        // Update
        if (sim || simonce)
        {
            //M_PROFILER_SCOPE(world_update);
            world.update();
            simonce = false;
        }
        {
            //M_PROFILER_SCOPE(vertices_update);

            for (unsigned i = 0; i < W*H; ++i)
            {
                sf::Vertex & vert = points[i];
                vert.color = world.at(i % W, i / W).getColor();
            }
        }


        // Render
        {
            //M_PROFILER_SCOPE(render_display);

            //win.clear(sf::Color::Black);
            win.draw(points);
            win.display();
        }

        ++frame;

        if (std::chrono::steady_clock::now() - begin > std::chrono::seconds(10))
        {
            begin = std::chrono::steady_clock::now();
            M_LOG_INFO(frame);
            frame = 0;
        }
    }

    //M_PROFILER_DUMP("profile.txt");

    return 0;
}*/