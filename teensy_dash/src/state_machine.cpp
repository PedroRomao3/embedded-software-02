#include "state_machine.hpp"

#include <io_settings.hpp>

StateMachine::StateMachine(CanCommHandler& can_handler, LogicHandler& logic_handler,
                           IOManager& io_manager)
    : can_handler(can_handler), logic_handler(logic_handler), io_manager(io_manager) {}

void StateMachine::update() {
  int torque_from_apps = 0;
  switch (current_state_) {
    case State::IDLE:
      if (logic_handler.should_start_manual_driving()) {
        current_state_ = State::INITIALIZING_DRIVING;
        io_manager.play_r2d_sound();  // tapem os ouvidos!
      } else if (logic_handler.should_start_as_driving()) {
        current_state_ = State::INITIALIZING_AS_DRIVING;
      } else {
        // Serial.println("chillin");
      }
      break;
    case State::INITIALIZING_DRIVING:
      if(transition_to_driving()){
        current_state_ = State::DRIVING;
      }
      break; // wait for transition to finish
    case State::INITIALIZING_AS_DRIVING:
      if(transition_to_driving()){
        current_state_ = State::AS_DRIVING;
      }
      break; // wait for transition to finish
    case State::DRIVING:
      torque_from_apps = logic_handler.calculate_torque();
      if (logic_handler.should_go_idle()) {
        current_state_ = State::IDLE;
        can_handler.stop_bamocar();
        return;
      }
      if (torque_from_apps == config::apps::ERROR_PLAUSIBILITY) {
        can_handler.stop_bamocar();
        current_state_ = State::IDLE;
        return;
      }
      can_handler.send_torque(torque_from_apps); /* VVVVVRRRRRRRRRRUUUUUUMMMMMMMMMMMMMMMMMMMMMMMm */
      break;
    case State::AS_DRIVING:
      if (logic_handler.should_go_idle()) {
        current_state_ = State::IDLE;
        can_handler.stop_bamocar();
      }
      if (logic_handler.just_entered_emergency()) {
        can_handler.stop_bamocar();
        current_state_ = State::IDLE;
        io_manager.play_buzzer(config::buzzer::EMERGENCY_DURATION);
      }
      break;
    default:
      break;
  }
}

bool StateMachine::transition_to_driving() const {
  const bool done = can_handler.init_bamocar();
  return done;
}
