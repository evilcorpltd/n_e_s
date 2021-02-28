#pragma once

#include "nes/core/imembank.h"
#include "nes/core/ines_controller.h"

namespace n_e_s::core {

class MemBankControllerIO : public IMemBank {
public:
    MemBankControllerIO(INesController *controller1,
            INesController *controller2)
            : controller1_(controller1), controller2_(controller2) {}

    bool is_address_in_range(uint16_t addr) const override {
        return (addr == controller1_addr_) || (addr == controller2_addr_);
    }

    uint8_t read_byte(uint16_t addr) const override {
        // Source: https://wiki.nesdev.com/w/index.php/Standard_controller
        // In the NES and Famicom, the top three (or five) bits are not
        // driven, and so retain the bits of the previous byte on the bus.
        // Usually this is the most significant byte of the address of the
        // controller port—0x40. Certain games (such as Paperboy) rely on
        // this behavior and require that reads from the controller ports
        // return exactly $40 or $41 as appropriate.

        INesController *controller{};
        uint8_t read_cnt{0};

        if (addr == controller1_addr_) {
            controller = controller1_;
            read_cnt = read_cnt1_++;
        } else if (addr == controller2_addr_) {
            controller = controller2_;
            read_cnt = read_cnt2_++;
        } else {
            return 0x00;
        }

        uint8_t data = 0x40;
        uint8_t btn = 0x00;

        // Special case, if strobe (0x4016, bit 1) is high, always return button
        // A for each controller
        if ((data1_ & static_cast<uint8_t>(0x01)) == 0x01) {
            btn = controller->get(INesController::Button::A) ? 0x01 : 0x00;
            return static_cast<uint8_t>(data & static_cast<uint8_t>(0xFE)) |
                   btn;
        }

        switch (read_cnt) {
        case 0:
            btn = controller->get(INesController::Button::A) ? 0x01 : 0x00;
            break;
        case 1:
            btn = controller->get(INesController::Button::B) ? 0x01 : 0x00;
            break;
        case 2:
            btn = controller->get(INesController::Button::Select) ? 0x01 : 0x00;
            break;
        case 3:
            btn = controller->get(INesController::Button::Start) ? 0x01 : 0x00;
            break;
        case 4:
            btn = controller->get(INesController::Button::Up) ? 0x01 : 0x00;
            break;
        case 5:
            btn = controller->get(INesController::Button::Down) ? 0x01 : 0x00;
            break;
        case 6:
            btn = controller->get(INesController::Button::Left) ? 0x01 : 0x00;
            break;
        case 7:
            btn = controller->get(INesController::Button::Right) ? 0x01 : 0x00;
            break;
        default:
            btn = 0x01; // After 8th read, return 1 for official Nintendo
                        // controllers
        }
        data = static_cast<uint8_t>(data & static_cast<uint8_t>(0xFE)) | btn;
        return data;
    }

    void write_byte(uint16_t addr, uint8_t data) override {
        if (addr == controller1_addr_) {
            if (((data1_ & static_cast<uint8_t>(0x01)) == 0x01) &&
                    ((data & static_cast<uint8_t>(0x01)) == 0x00)) {
                // Neg flank on bit 0 should latch controller button states -
                // here just make sure counters is reset
                read_cnt1_ = 0;
                read_cnt2_ = 0;
            }
            data1_ = data;
        } else if (addr == controller2_addr_) {
            data2_ = data;
        }
    }

private:
    INesController *controller1_;
    INesController *controller2_;
    mutable uint8_t read_cnt1_{0};
    mutable uint8_t read_cnt2_{0};
    uint8_t data1_{0};
    uint8_t data2_{0};

    const uint16_t controller1_addr_{0x4016};
    const uint16_t controller2_addr_{0x4017};
}; // namespace n_e_s::core

} // namespace n_e_s::core