#include "app.h"
#include <emscripten.h>

namespace
{
    constexpr int FPS = 10;
    constexpr int DIM = 20;

    EM_JS(int, getWidth, (), {
        return window.innerWidth;
    });

    EM_JS(int, getHeight, (), {
        return window.innerHeight;
    });

    void mainLoop(void *app)
    {
        static_cast<App *>(app)->drawLoop();
    }
}

App::~App()
{
    SDL_DestroyRenderer(renderer_);
    SDL_DestroyWindow(window_);
}

void App::start()
{
    width_ = getWidth();
    height_ = getHeight();

    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(width_, height_, 0, &window_, &renderer_);
    emscripten_set_main_loop_arg(mainLoop, this, FPS, 1);
}

void App::drawLoop()
{
    handleEvents();

    if (paused_)
        return;

    checkSize();

    auto &[solved, x, y] = step_;
    if (!done_)
    {
        initMaze();
        solved = false;
    }

    drawMaze();

    if (solved)
        drawLightningAnimation();
    else
        drawProgress();
}

void App::handleEvents()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_QUIT:
            emscripten_cancel_main_loop();
            break;

        case SDL_KEYDOWN:
            // TODO: Make this a pause and also add another key for single step.
            if (event.key.keysym.sym == SDLK_p)
                paused_ = !paused_;
            break;
        }
    }
}

void App::checkSize()
{
    auto width = getWidth();
    auto height = getHeight();
    if (width != width_ || height != height_)
    {
        width_ = width;
        height_ = height;
        emscripten_cancel_main_loop();
        SDL_SetWindowSize(window_, width_, height_);
        emscripten_set_main_loop_arg(mainLoop, this, FPS, 1);
    }
}

void App::initMaze()
{
    done_ = true;
    maze_.init((width_ - 1) / DIM, (height_ - 1) / DIM);
    maze_.start_solve();
    path_.clear();
    rects_.clear();
    bfs_progress_.clear();
}

void App::drawMaze()
{
    SDL_SetRenderDrawColor(renderer_, 0x00, 0x80, 0x00, 0xFF);

    for (unsigned y = 0; y <= maze_.height(); ++y)
    {
        unsigned yy = y * DIM;
        for (unsigned x = 0; x <= maze_.width(); ++x)
        {
            unsigned xx = x * DIM;
            if (y < maze_.height() && maze_.getVwall(x, y))
            {
                SDL_RenderDrawLine(renderer_, xx, yy, xx, yy + DIM);
            }
            if (x < maze_.width() && maze_.getHwall(x, y))
            {
                SDL_RenderDrawLine(renderer_, xx, yy, xx + DIM, yy);
            }
        }
    }
    SDL_RenderPresent(renderer_);
}

void App::drawLightningAnimation()
{
    auto &[solved, x, y] = step_;
    if (path_.empty())
    {
        auto a = std::back_inserter(path_);
        maze_.back_track(x, y, a);
        for (const auto &a : path_)
        {
            rects_.push_back({(int)a.second * DIM, (int)a.first * DIM, DIM, DIM});
        }
        animation_step_ = 0;
    }

    if (animation_step_ < 10 || (animation_step_ > 15 && animation_step_ < 18) || animation_step_ > 20)
    {
        SDL_SetRenderDrawColor(renderer_, 0x00, 0x80, 0x00, 0xFF);
        for (const auto &a : rects_)
        {
            SDL_RenderFillRect(renderer_, &a);
        }
        SDL_RenderPresent(renderer_);
    }

    animation_step_++;
    if (animation_step_ > 25)
    {
        done_ = false;
    }
    return;
}

void App::drawProgress()
{
    auto &[solved, x, y] = step_;
    size_t n = maze_.queueSize();
    if (n == 0)
    {
        done_ = false;
        return;
    }

    while (n-- > 0 && !solved)
    {
        step_ = maze_.solve_step();
        bfs_progress_[std::make_pair(x, y)] = std::make_pair(5, SDL_Rect{(int)x * DIM, (int)y * DIM, DIM, DIM});
    }

    // draw progess, then degrade
    auto iter = bfs_progress_.begin();
    while (iter != bfs_progress_.end())
    {
        if (--iter->second.first == 0)
        {
            bfs_progress_.erase(iter++);
            continue;
        }
        SDL_SetRenderDrawColor(renderer_, 0x00, 0x20 * iter->second.first, 0x00, 0xFF);
        SDL_RenderFillRect(renderer_, &iter->second.second);
        iter++;
    }
}