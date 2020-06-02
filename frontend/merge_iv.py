#!/usr/bin/python3

def _intersect(interval1, interval2):
    """
    Find whether two intervals intersect
    :param interval1: list [a, b], where 'a' is the left border and 'b' is the right border
    :param interval2: list [c, d], where 'c' is the left border and 'd' is the right border
    :return: True if intervals intersect, False otherwise
    """
    if interval1[0] < interval2[0]:
        left = interval1
        right = interval2
    elif interval2[0] < interval1[0]:
        left = interval2
        right = interval1
    else:  # so interval1[0] == interval2[0]
        return True

    if left[1] >= right[0]:
        return True
    else:
        return False

def _merge(interval1, interval2):
    """
    Finds merge of two intersecting intervals. This function should be called only if it's checked that
    intervals intersect, e.g. if "_intersect(interval1, interval2)" is True

    :param interval1: list [a, b], where 'a' is the left border and 'b' is the right border
    :param interval2: list [c, d], where 'c' is the left border and 'd' is the right border
    :return: new interval that contains only both intervals
    """
    return [min(interval1[0], interval2[0]), max(interval1[1], interval2[1])]

def merge_iv(intervals, interval_to_add):
    """
    Adds 'interval_to_add' into 'intervals' list:
    1) as a separate list if 'interval_to_add' intersects with no one in 'intervals' list
    2) as a product of merging with intervals that 'interval_to_add' intersect

    :param intervals: list of some existing intervals, e.g. [[a, b], [c, d], ...]
    :param interval_to_add: interval that must be added to 'intervals' list considering that it can intersect
    with some of intervals in 'intervals' list and then they must be merged into one bigger interval
    :return: list of intervals after adding 'interval_to_add' to 'intervals' with possible merge
    """
    new_intervals = []
    for segment in intervals:
        if _intersect(segment, interval_to_add):
            interval_to_add = _merge(segment, interval_to_add)
            continue
        new_intervals.append(segment)

    new_intervals.append(interval_to_add)
    return new_intervals
