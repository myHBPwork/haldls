#!/usr/bin/env python

import unittest
import pyhalco_common as Co
import pyhalco_hicann_dls_v2 as C
import pyhaldls_v2 as Ct
import pystadls_v2 as IO

import pylogging as logger


class TestPyhaldlsIOV2(unittest.TestCase):
    def test_program_builder(self):
        builder = Ct.PlaybackProgramBuilder()
        builder.write(C.CapMemOnDLS(), Ct.CapMem())
        builder.wait_until(100)
        capmem_ticket = builder.read(C.CapMemOnDLS())
        cell = C.CapMemCellOnDLS(Co.Enum(2))
        capmemcell_ticket = builder.read(cell)
        builder.halt()

        program = builder.done()

        # Builder is reset (starts a new program) every time .done() is called
        empty_new_program = builder.done()
        self.assertNotEqual(program.serial_number(), empty_new_program.serial_number())

        # No data available yet
        with self.assertRaises(RuntimeError):
            capmem_copy = program.get(capmem_ticket)
        with self.assertRaises(RuntimeError):
            capmemcell_copy = program.get(capmemcell_ticket)

        capmem_ticket_ = builder.read(C.CapMemOnDLS())
        program_ = builder.done()

        # Using Ticket issued for a different program
        with self.assertRaises(ValueError):
            capmem_copy = program.get(capmem_ticket_)
        with self.assertRaises(ValueError):
            capmem_copy = program_.get(capmem_ticket)


if __name__ == "__main__":
    logger.reset()
    logger.default_config(level=logger.LogLevel.DEBUG)
    unittest.main()
