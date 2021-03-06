// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for AT&T 610 Business Communication Terminal (BCT).

    The video ASIC is a 68-pin PLCC marked:

        (M) AT&T
        1006B2*
        456555
        18289S 71

    Another custom IC (28-pin DIP) is next to the character generator:

           WE
         492A*
        18186 71

****************************************************************************/

#include "emu.h"

//#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "machine/mc68681.h"
#include "machine/z80ctc.h"
#include "machine/z80sio.h"
#include "screen.h"

class att610_state : public driver_device
{
public:
	att610_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_rom(*this, "rom")
	{
	}

	void att610(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void cart_select_w(u8 data);

	void mem_map(address_map &map);
	void io_map(address_map &map);

	required_device<z80_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_memory_bank m_rom;
};

void att610_state::machine_start()
{
	m_rom->configure_entry(0, memregion("firmware")->base());
	m_rom->configure_entry(1, memregion("cartridge")->base());
}

void att610_state::machine_reset()
{
	m_rom->set_entry(0);
}

u32 att610_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void att610_state::cart_select_w(u8 data)
{
	m_rom->set_entry(BIT(data, 0));
}

void att610_state::mem_map(address_map &map)
{
	map(0x0000, 0x5fff).bankr("rom");
	map(0xc000, 0xffff).ram();
}

void att610_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x03).rw("sio", FUNC(z80sio_device::cd_ba_r), FUNC(z80sio_device::cd_ba_w));
	map(0x10, 0x10).w(FUNC(att610_state::cart_select_w));
	map(0x60, 0x63).rw("ctc", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x70, 0x7f).rw("duart", FUNC(scn2681_device::read), FUNC(scn2681_device::write));
}


static INPUT_PORTS_START(att610)
	// TODO: supports 103-key (56K430/ACZ) and 98-key (56K420/ADA) keyboards
INPUT_PORTS_END

static const z80_daisy_config daisy_chain[] =
{
	{ "sio" },
	{ "ctc" },
	{ nullptr }
};

void att610_state::att610(machine_config &config)
{
	Z80(config, m_maincpu, 27.72_MHz_XTAL / 7); // MK3880N-4; CPU clock guessed
	m_maincpu->set_addrmap(AS_PROGRAM, &att610_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &att610_state::io_map);
	m_maincpu->set_daisy_config(daisy_chain);

	z80ctc_device &ctc(Z80CTC(config, "ctc", 27.72_MHz_XTAL / 7)); // Z8430APS
	ctc.intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	z80sio_device &sio(Z80SIO(config, "sio", 27.72_MHz_XTAL / 7)); // Z8441APS (SIO/1)
	sio.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	SCN2681(config, "duart", 3'686'400); // MC2681P (adjacent XTAL not legible)

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER, rgb_t::green());
	m_screen->set_raw(21.6675_MHz_XTAL, 963, 0, 720, 375, 0, 351);
	//m_screen->set_raw(27.72_MHz_XTAL, 1232, 0, 924, 375, 0, 351);
	m_screen->set_screen_update(FUNC(att610_state::screen_update));
}

ROM_START(att610)
	ROM_REGION(0x6000, "firmware", 0)
	ROM_LOAD("455798-1.d4", 0x0000, 0x2000, CRC(4a704dbe) SHA1(728bf1e5edacfb9749f795e3c2fd58cef9f509d3) BAD_DUMP) // MBM27128-25 (dumped half size)
	ROM_LOAD("455799-1.b4", 0x4000, 0x2000, CRC(7fd75ee0) SHA1(597b23c43b3f283b49b51b9dee60109ff683b041)) // MBM2764-25

	ROM_REGION(0x6000, "cartridge", ROMREGION_ERASE00) // optional

	ROM_REGION(0x2000, "chargen", 0)
	ROM_LOAD("att-tc85_456309-1.h7", 0x0000, 0x2000, CRC(d313e022) SHA1(a24df1d8d8c55413e4cdb0734783c0fa244bdf00)) // HN27C64G-15
ROM_END

COMP(1985, att610, 0, 0, att610, att610, att610_state, empty_init, "AT&T", "610 Business Communication Terminal", MACHINE_IS_SKELETON)
