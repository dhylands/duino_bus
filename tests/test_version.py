#!/usr/bin/env python3

import unittest

from duino_bus.version import __version__


class TestPacketDecoder(unittest.TestCase):

    def test_version(self):
        # This test is just to eliminate version.py from the coverage report
        self.assertNotEqual(__version__, '')
