/*
 * SPDX-FileCopyrightText: 2017-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

using MessagePack;

namespace SensCord
{
    /// <summary>
    /// Vector2.
    /// </summary>
    /// <typeparam name="T">Generic type parameter.</typeparam>
    [MessagePackObject]
    public class Vector2<T>
    {
        /// <summary>
        /// X.
        /// </summary>
        [Key("x")]
        public T x { get; set; }

        /// <summary>
        /// Y.
        /// </summary>
        [Key("y")]
        public T y { get; set; }

        /// <summary>
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return $"Vector2({this.x}, {this.y})";
        }
    }

    /// <summary>
    /// Vector3.
    /// </summary>
    /// <typeparam name="T">Generic type parameter.</typeparam>
    [MessagePackObject]
    public class Vector3<T>
    {
        /// <summary>
        /// X.
        /// </summary>
        [Key("x")]
        public T x { get; set; }

        /// <summary>
        /// Y.
        /// </summary>
        [Key("y")]
        public T y { get; set; }

        /// <summary>
        /// Z.
        /// </summary>
        [Key("z")]
        public T z { get; set; }

        /// <summary>
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return $"Vector3({this.x}, {this.y}, {this.z})";
        }
    }

    /// <summary>
    /// Vector4.
    /// </summary>
    /// <typeparam name="T">Generic type parameter.</typeparam>
    [MessagePackObject]
    public class Vector4<T>
    {
        /// <summary>
        /// X.
        /// </summary>
        [Key("x")]
        public T x { get; set; }

        /// <summary>
        /// Y.
        /// </summary>
        [Key("y")]
        public T y { get; set; }

        /// <summary>
        /// Z.
        /// </summary>
        [Key("z")]
        public T z { get; set; }

        /// <summary>
        /// A.
        /// </summary>
        [Key("a")]
        public T a { get; set; }

        /// <summary>
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return $"Vector4({this.x}, {this.y}, {this.z}, {this.a})";
        }
    }

    /// <summary>
    /// Quaternion.
    /// </summary>
    /// <typeparam name="T">Generic type parameter.</typeparam>
    [MessagePackObject]
    public class Quaternion<T>
    {
        /// <summary>
        /// X.
        /// </summary>
        [Key("x")]
        public T x { get; set; }

        /// <summary>
        /// Y.
        /// </summary>
        [Key("y")]
        public T y { get; set; }

        /// <summary>
        /// Z.
        /// </summary>
        [Key("z")]
        public T z { get; set; }

        /// <summary>
        /// W.
        /// </summary>
        [Key("w")]
        public T w { get; set; }

        /// <summary>
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return $"Quaternion({this.x}, {this.y}, {this.z}, {this.w})";
        }
    }

    /// <summary>
    /// Matrix 3x3.
    /// </summary>
    /// <typeparam name="T">Generic type parameter.</typeparam>
    [MessagePackObject]
    public class Matrix<T>
    {
        /// <summary>
        /// Element.
        /// </summary>
        [Key("element")]
        public T[][] Element { get; set; } = new T[3][]
        {
            new T[3],
            new T[3],
            new T[3],
        };

        /// <summary>
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return string.Format(
                "Matrix([{0}], [{1}], [{2}])",
                string.Join(", ", this.Element[0]),
                string.Join(", ", this.Element[1]),
                string.Join(", ", this.Element[2]));
        }
    }

    /// <summary>
    /// Matrix 3x3.
    /// </summary>
    /// <typeparam name="T">Generic type parameter.</typeparam>
    [MessagePackObject]
    public class Matrix3x3<T>
    {
        /// <summary>
        /// Element.
        /// </summary>
        [Key("element")]
        public T[][] Element { get; set; } = new T[3][]
        {
            new T[3],
            new T[3],
            new T[3],
        };

        /// <summary>
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return string.Format(
                "Matrix3x3([{0}], [{1}], [{2}])",
                string.Join(", ", this.Element[0]),
                string.Join(", ", this.Element[1]),
                string.Join(", ", this.Element[2]));
        }
    }

    /// <summary>
    /// Matrix 3x4.
    /// </summary>
    /// <typeparam name="T">Generic type parameter.</typeparam>
    [MessagePackObject]
    public class Matrix3x4<T>
    {
        /// <summary>
        /// Element.
        /// </summary>
        [Key("element")]
        public T[][] Element { get; set; } = new T[3][]
        {
            new T[4],
            new T[4],
            new T[4],
        };

        /// <summary>
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return string.Format(
                "Matrix([{0}], [{1}], [{2}])",
                string.Join(", ", this.Element[0]),
                string.Join(", ", this.Element[1]),
                string.Join(", ", this.Element[2]));
        }
    }

    /// <summary>
    /// Scalar.
    /// </summary>
    /// <typeparam name="T">Generic type parameter.</typeparam>
    [MessagePackObject]
    public class Scalar<T>
    {
        /// <summary>
        /// Value.
        /// </summary>
        [Key("value")]
        public T value { get; set; }

        /// <summary>
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return $"Scalar({this.value})";
        }
    }

    /// <summary>
    ///  Range expressed by the min max.
    /// </summary>
    /// <typeparam name="T">Generic type parameter.</typeparam>
    [MessagePackObject]
    public class Range<T>
    {
        /// <summary>
        /// Min.
        /// </summary>
        [Key("min")]
        public T min { get; set; }

        /// <summary>
        /// Max.
        /// </summary>
        [Key("max")]
        public T max { get; set; }

        /// <summary>
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return $"Range(min={this.min}, max={this.max})";
        }
    }

    /// <summary>
    /// Misalignment of the axis direction.
    /// </summary>
    [MessagePackObject]
    public class AxisMisalignment
    {
        /// <summary>
        /// Ms.
        /// </summary>
        [Key("ms")]
        public Matrix3x3<float> MS { get; set; } = new Matrix3x3<float>();

        /// <summary>
        /// Offset.
        /// </summary>
        [Key("offset")]
        public Vector3<float> Offset { get; set; } = new Vector3<float>();

        /// <summary>
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return $"AxisMisalignment({this.MS}, {this.Offset})";
        }
    }
}
