#pragma once


template <class Iterator>
class window {
    Iterator m_begin, m_end;

public:
    auto begin() const {return m_begin;}
    auto end() const {return m_end;}

    window & operator++() {++m_begin;}
    operator bool() const {return m_begin != m_end;}
};
