#include "InputManager.hpp"

#include <GLFW/glfw3.h>

namespace RtEngine
{
    InputManager::InputManager(const std::shared_ptr<Window>& window) : window(window)
    {
        window->addKeyCallback(
            [this](int key, int scancode, int action, int mods) { processGlfwKeyEvent(key, action); });
        window->addMouseCallback([this](double x_pos, double y_pos) { processGlfwMouseEvent(x_pos, y_pos); });
    }

    bool InputManager::getKeyDown(const Keycode key) const
    {
        return down_keycodes.contains(key);
    }

    bool InputManager::getKeyUp(const Keycode key) const
    {
        return up_keycodes.contains(key);
    }

    glm::vec2 InputManager::getMousePosition() const
    {
        return mouse_pos;
    }

    void InputManager::reset()
    {
        down_keycodes.clear();
        up_keycodes.clear();
    }

    void InputManager::processGlfwKeyEvent(int key, int action)
    {
        if (action == GLFW_PRESS)
        {
            down_keycodes.insert(glfwToEngineKeycode(key));
        }

        if (action == GLFW_RELEASE)
        {
            up_keycodes.insert(glfwToEngineKeycode(key));
        }
    }

    void InputManager::processGlfwMouseEvent(double x_pos, double y_pos)
    {
        mouse_pos.x = x_pos;
        mouse_pos.y = y_pos;
    }

    Keycode InputManager::glfwToEngineKeycode(int glfw_key)
    {
        switch (glfw_key)
        {
        case GLFW_KEY_A:
            return Keycode::A;
        case GLFW_KEY_B:
            return Keycode::B;
        case GLFW_KEY_C:
            return Keycode::C;
        case GLFW_KEY_D:
            return Keycode::D;
        case GLFW_KEY_E:
            return Keycode::E;
        case GLFW_KEY_F:
            return Keycode::F;
        case GLFW_KEY_G:
            return Keycode::G;
        case GLFW_KEY_H:
            return Keycode::H;
        case GLFW_KEY_I:
            return Keycode::I;
        case GLFW_KEY_J:
            return Keycode::J;
        case GLFW_KEY_K:
            return Keycode::K;
        case GLFW_KEY_L:
            return Keycode::L;
        case GLFW_KEY_M:
            return Keycode::M;
        case GLFW_KEY_N:
            return Keycode::N;
        case GLFW_KEY_O:
            return Keycode::O;
        case GLFW_KEY_P:
            return Keycode::P;
        case GLFW_KEY_Q:
            return Keycode::Q;
        case GLFW_KEY_R:
            return Keycode::R;
        case GLFW_KEY_S:
            return Keycode::S;
        case GLFW_KEY_T:
            return Keycode::T;
        case GLFW_KEY_U:
            return Keycode::U;
        case GLFW_KEY_V:
            return Keycode::V;
        case GLFW_KEY_W:
            return Keycode::W;
        case GLFW_KEY_X:
            return Keycode::X;
        case GLFW_KEY_Y:
            return Keycode::Y;
        case GLFW_KEY_Z:
            return Keycode::Z;

        case GLFW_KEY_0:
            return Keycode::NUM_0;
        case GLFW_KEY_1:
            return Keycode::NUM_1;
        case GLFW_KEY_2:
            return Keycode::NUM_2;
        case GLFW_KEY_3:
            return Keycode::NUM_3;
        case GLFW_KEY_4:
            return Keycode::NUM_4;
        case GLFW_KEY_5:
            return Keycode::NUM_5;
        case GLFW_KEY_6:
            return Keycode::NUM_6;
        case GLFW_KEY_7:
            return Keycode::NUM_7;
        case GLFW_KEY_8:
            return Keycode::NUM_8;
        case GLFW_KEY_9:
            return Keycode::NUM_9;

        case GLFW_KEY_SPACE:
            return Keycode::SPACE;
        case GLFW_KEY_LEFT_SHIFT:
            return Keycode::SHIFT;
        case GLFW_KEY_RIGHT_SHIFT:
            return Keycode::SHIFT;

        default:
            return Keycode::UNKNOWN;
        }
    }
} // namespace RtEngine