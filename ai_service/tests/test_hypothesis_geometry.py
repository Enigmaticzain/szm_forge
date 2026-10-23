"""Hypothesis property tests for geometry invariants (Group 17)."""
from __future__ import annotations

import pytest

hypothesis = pytest.importorskip("hypothesis")
from hypothesis import given, settings
from hypothesis import strategies as st


@given(
    length=st.floats(min_value=0.01, max_value=10.0, allow_nan=False, allow_infinity=False),
    width=st.floats(min_value=0.01, max_value=10.0, allow_nan=False, allow_infinity=False),
    height=st.floats(min_value=0.01, max_value=10.0, allow_nan=False, allow_infinity=False),
)
@settings(max_examples=20, deadline=None)
def test_box_volume_positive(length, width, height):
    volume = length * width * height
    assert volume > 0


@given(
    force=st.floats(min_value=1.0, max_value=1e6, allow_nan=False, allow_infinity=False),
    area=st.floats(min_value=1e-6, max_value=10.0, allow_nan=False, allow_infinity=False),
)
@settings(max_examples=20, deadline=None)
def test_stress_force_over_area(force, area):
    stress = force / area
    assert stress > 0
    assert stress == pytest.approx(force / area)
