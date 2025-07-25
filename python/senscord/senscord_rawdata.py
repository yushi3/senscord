# SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
#
# SPDX-License-Identifier: Apache-2.0

################################################################################

"""Raw data of SensCord SDK."""

from __future__ import absolute_import

from senscord import serialize
from senscord import senscord_types as _types


class AccelerationData(_types.Vector3f):
    """Acceleration data."""


class AngularVelocityData(_types.Vector3f):
    """Angular velocity data."""


class MagneticFieldData(_types.Vector3f):
    """Magnetic field data."""


class RotationData(serialize.Structure):
    """Rotation data."""
    _fields_ = [
        ('roll', serialize.Float),
        ('pitch', serialize.Float),
        ('yaw', serialize.Float),
    ]
    #: :var float: Roll angle.
    roll = None
    #: :var float: Pitch angle.
    pitch = None
    #: :var float: Yaw angle.
    yaw = None

    def __repr__(self):
        return '%s(roll=%r, pitch=%r, yaw=%r)' % (
            self.__class__.__name__, self.roll, self.pitch, self.yaw)


class PoseQuaternionData(serialize.Structure):
    """Pose quaternion data."""
    _fields_ = [
        ('position', _types.Vector3f),
        ('orientation', _types.Quaternionf),
    ]
    #: :var Vector3f: Position(x,y,z)
    position = None
    #: :var Quaternionf: Qrientation(x,y,z,w)
    orientation = None

    def __repr__(self):
        return '%s(position=%r, orientation=%r)' % (
            self.__class__.__name__, self.position, self.orientation)


class PoseData(PoseQuaternionData):
    """Pose data."""


class PoseMatrixData(serialize.Structure):
    """Pose matrix data."""
    _fields_ = [
        ('position', _types.Vector3f),
        ('rotation', _types.Matrix3x3f),
    ]
    #: :var Vector3f: Position(x,y,z)
    position = None
    #: :var Matrix3x3f: Matrix(3x3)
    rotation = None

    def __repr__(self):
        return '%s(position=%r, rotation=%r)' % (
            self.__class__.__name__, self.position, self.rotation)


class RectangleRegionParameter(serialize.Structure):
    """Structure for region of rectangle."""
    _fields_ = [
        ('top', serialize.Uint32),
        ('left', serialize.Uint32),
        ('bottom', serialize.Uint32),
        ('right', serialize.Uint32),
    ]
    top = None      #: :var Uint32: upper position of region from origin
    left = None     #: :var Uint32: left  position of region from origin
    bottom = None   #: :var Uint32: bottom position of region from origin
    right = None    #: :var Uint32: right position of region from origin

    def __repr__(self):
        return '%s(top=%r, left=%r, bottom=%r, right=%r)' % (
            self.__class__.__name__, self.top, self.left,
            self.bottom, self.right)


class DetectedObjectInformation(serialize.Structure):
    """Detected Object Information."""
    _fields_ = [
        ('class_id', serialize.Uint32),
        ('score', serialize.Float),
        ('box', RectangleRegionParameter),
    ]
    #: :var Uint32: Class id of detected object
    class_id = None
    #: :var Float: Score of detected object
    score = None
    #: :var RectangleRegionParameter: Detected object area
    box = None

    def __repr__(self):
        return '%s(class_id=%r, score=%r, box=%r)' % (
            self.__class__.__name__, self.class_id, self.score, self.box)


class ObjectDetectionData(serialize.Structure):
    """Object Detection Data."""
    _fields_ = [
        ('data', (list, DetectedObjectInformation)),
    ]
    #: :var list: Detected objects.
    data = None

    def __repr__(self):
        return '%s(data=%r)' % (
            self.__class__.__name__, self.data)


class KeyPoint(serialize.Structure):
    """Key Point."""
    _fields_ = [
        ('key_point_id', serialize.Uint32),
        ('score', serialize.Float),
        ('point', _types.Vector3f),
    ]
    #: :var Uint32: Key point id of detected object.
    key_point_id = None
    #: :var Float: Score of detected object.
    score = None
    #: :var List: Point coordinates.
    point = None

    def __repr__(self):
        return '%s(key_point_id=%r, score=%r, point=%r)' % (
            self.__class__.__name__, self.key_point_id, self.score, self.point)


class DetectedKeyPointInformation(serialize.Structure):
    """Detected Key Point Information."""
    _fields_ = [
        ('class_id', serialize.Uint32),
        ('score', serialize.Float),
        ('key_points', (list, KeyPoint)),
    ]
    #: :var Uint32: Class id of detected object.
    class_id = None
    #: :var Float: Score of detected object.
    score = None
    #: :var List: Detected points.
    key_points = None

    def __repr__(self):
        return '%s(class_id=%r, score=%r, key_points=%r)' % (
            self.__class__.__name__, self.class_id, self.score, self.key_points)


class KeyPointData(serialize.Structure):
    """Key Point Data."""
    _fields_ = [
        ('data', (list, DetectedKeyPointInformation)),
    ]
    #: :var list: Detected points.
    data = None

    def __repr__(self):
        return '%s(data=%r)' % (
            self.__class__.__name__, self.data)


class TrackedObjectInformation(serialize.Structure):
    """Tracked Object Information."""
    _fields_ = [
        ('track_id', serialize.Uint32),
        ('class_id', serialize.Uint32),
        ('score', serialize.Float),
        ('velocity', _types.Vector2f),
        ('position', _types.Vector2u32),
        ('box', RectangleRegionParameter),
    ]
    #: :var Uint32: Track id of tracked object
    track_id = None
    #: :var Uint32: Class id of tracked object
    class_id = None
    #: :var Float: Score of tracked object
    score = None
    #: :var Vector2f: Velocity(x,y) of tracked object
    velocity = None
    #: :var Vector2u32: Position(x,y) of tracked object
    position = None
    #: :var RectangleRegionParameter: Tracked object area
    box = None

    def __repr__(self):
        return ('%s(track_id=%r, class_id=%r, score=%r,'
                'velocity=%r, position=%r, box=%r)') % (
                    self.__class__.__name__, self.track_id, self.class_id,
                    self.score, self.velocity, self.position, self.box)


class ObjectTrackingData(serialize.Structure):
    """Object Tracking Data."""
    _fields_ = [
        ('data', (list, TrackedObjectInformation)),
    ]
    #: :var list: Detected objects.
    data = None

    def __repr__(self):
        return '%s(data=%r)' % (
            self.__class__.__name__, self.data)
