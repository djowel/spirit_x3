#pragma once

namespace x4 {

template <class Iterator>
class window {
    Iterator m_begin, m_end;
    bool good;

public:
    auto begin() const {return m_begin;}
    auto end() const {return m_end;}

    window & operator++() {++m_begin;}
    operator bool() const {return m_begin != m_end;}
};

}


auto check(window)
--> return data if necessary
--> return whether check() was satisfied
--> return where iterator goes to after it

left.check()
window.check(left)
---> does check, modifies window if true, 
