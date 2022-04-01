#pragma once

namespace solver {
template<typename T> class state_saver
{
  public:
    explicit state_saver(T &state) : m_saved_state(state), m_saved_ref(&state) {}
    state_saver(const state_saver &) = delete;
    state_saver(state_saver &&) = default;
    ~state_saver() { *m_saved_ref = m_saved_state; }
    state_saver &operator=(const state_saver &) = delete;
    const T &saved_state() const { return m_saved_state; }

  private:
    T m_saved_state;
    T *m_saved_ref;
};
}// namespace solver