#pragma once

#include <vector>

template<class T>
class TwoDWrapper
{   
    std::vector<T> m_Items;
    int m_Width;
    int m_Height;

public:
    TwoDWrapper() = default;
    TwoDWrapper(int w, int h) : m_Width(w), m_Height(h), m_Items(w * h) {}
    
    const T& get(int x, int y) const
    {
        return m_Items[x + y * m_Width];
    }

    T& get(int x, int y)
    {
        return m_Items[x + y * m_Width];
    }

};