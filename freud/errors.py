# Copyright (c) 2010-2019 The Regents of the University of Michigan
# This file is from the freud project, released under the BSD 3-Clause License.

# \package freud.errors
#
# Errors and exceptions internal to freud


class FreudDeprecationWarning(UserWarning):
    """Raised when a freud feature is pending deprecation."""
    pass
