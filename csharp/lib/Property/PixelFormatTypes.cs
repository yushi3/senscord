/*
 * SPDX-FileCopyrightText: 2017-2024 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

namespace SensCord
{
    /// <summary>
    /// Pixel format.
    /// </summary>
    public static partial class PixelFormat
    {
        #region Packed RGB

        /// <summary>
        /// The ARGB 4444.
        /// </summary>
        public static readonly string ARGB444 = "image_argb444";
        //// @deprecated public static readonly string IMAGE_ARGB444 = "image_argb444";

        /// <summary>
        /// The XRGB 4444.
        /// </summary>
        public static readonly string XRGB444 = "image_xrgb444";
        //// @deprecated public static readonly string IMAGE_XRGB444 = "";

        /// <summary>
        /// The RGB 888.
        /// </summary>
        public static readonly string RGB24 = "image_rgb24";
        //// @deprecated public static readonly string IMAGE_RGB24 = "image_rgb24";

        /// <summary>
        /// The ARGB 8888.
        /// </summary>
        public static readonly string ARGB32 = "image_argb32";
        //// @deprecated public static readonly string IMAGE_ARGB32 = "image_argb32";

        /// <summary>
        /// The XRGB 8888.
        /// </summary>
        public static readonly string XRGB32 = "image_xrgb32";
        //// @deprecated public static readonly string IMAGE_XRGB32 = "image_xrgb32";

        /// <summary>
        /// The BGR 888.
        /// </summary>
        public static readonly string BGR24 = "image_bgr24";
        //// @deprecated public static readonly string IMAGE_BGR24 = "image_bgr24";

        /// <summary>
        /// The ABGR 8888.
        /// </summary>
        public static readonly string ABGR32 = "image_abgr32";
        //// @deprecated public static readonly string IMAGE_ABGR32 = "image_abgr32";

        /// <summary>
        /// The XBGR 8888.
        /// </summary>
        public static readonly string XBGR32 = "image_xbgr32";
        //// @deprecated public static readonly string IMAGE_XBGR32 = "image_xbgr32";

        #endregion

        #region Planar RGB

        /// <summary>
        /// The RGB8 Planar.
        /// </summary>
        public static readonly string RGB8Planar = "image_rgb8_planar";
        //// @deprecated public static readonly string IMAGE_RGB8_PLANAR = "image_rgb8_planar";

        /// <summary>
        /// The RGB16 Planar.
        /// </summary>
        public static readonly string RGB16Planar = "image_rgb16_planar";
        //// @deprecated public static readonly string IMAGE_RGB16_PLANAR = "image_rgb16_planar";

        #endregion

        #region Greyscale

        /// <summary>
        /// The 8-bit Greyscale.
        /// </summary>
        public static readonly string GREY = "image_grey";
        //// @deprecated public static readonly string IMAGE_GREY = "image_grey";

        /// <summary>
        /// The 10-bit Greyscale (on 16bit).
        /// </summary>
        public static readonly string Y10 = "image_y10";
        //// @deprecated public static readonly string IMAGE_Y10 = "image_y10";

        /// <summary>
        /// The 12-bit Greyscale (on 16bit).
        /// </summary>
        public static readonly string Y12 = "image_y12";
        //// @deprecated public static readonly string IMAGE_Y12 = "image_y12";

        /// <summary>
        /// The 14-bit Greyscale (on 16bit).
        /// </summary>
        public static readonly string Y14 = "image_y14";
        //// @deprecated public static readonly string IMAGE_Y14 = "image_y14";

        /// <summary>
        /// The 16-bit Greyscale.
        /// </summary>
        public static readonly string Y16 = "image_y16";
        //// @deprecated public static readonly string IMAGE_Y16 = "image_y16";

        /// <summary>
        /// The 20-bit Greyscale (on 32bit).
        /// </summary>
        public static readonly string Y20 = "image_y20";
        //// @deprecated public static readonly string IMAGE_Y20 = "image_y20";

        /// <summary>
        /// The 24-bit Greyscale (on 32bit).
        /// </summary>
        public static readonly string Y24 = "image_y24";
        //// @deprecated public static readonly string IMAGE_Y24 = "image_y24";

        #endregion

        #region YUV

        /// <summary>
        /// The YUV444.
        /// </summary>
        public static readonly string YUV444 = "image_yuv444";
        //// @deprecated public static readonly string IMAGE_YUV444 = "image_yuv444";

        /// <summary>
        /// The YUV420SP.
        /// </summary>
        public static readonly string NV12 = "image_nv12";
        //// @deprecated public static readonly string IMAGE_NV12 = "image_nv12";

        /// <summary>
        /// The YUV422SP.
        /// </summary>
        public static readonly string NV16 = "image_nv16";
        //// @deprecated public static readonly string IMAGE_NV16 = "image_nv16";

        /// <summary>
        /// The YUV420.
        /// </summary>
        public static readonly string YUV420 = "image_yuv420";
        //// @deprecated public static readonly string IMAGE_YUV420 = "image_yuv420";

        /// <summary>
        /// The YUV422P.
        /// </summary>
        public static readonly string YUV422P = "image_yuv422p";
        //// @deprecated public static readonly string IMAGE_YUV422P = "image_yuv422p";

        /// <summary>
        /// The YUYV.
        /// </summary>
        public static readonly string YUYV = "image_yuyv";
        //// @deprecated public static readonly string IMAGE_YUYV = "image_yuyv";

        /// <summary>
        /// The UYVY.
        /// </summary>
        public static readonly string UYVY = "image_uyvy";
        //// @deprecated public static readonly string IMAGE_UYVY = "image_uyvy";

        #endregion

        #region Bayer

        /// <summary>
        /// The SBGGR8.
        /// </summary>
        public static readonly string SBGGR8 = "image_sbggr8";
        //// @deprecated public static readonly string IMAGE_SBGGR8 = "image_sbggr8";

        /// <summary>
        /// The SGBRG8.
        /// </summary>
        public static readonly string SGBRG8 = "image_sgbrg8";
        //// @deprecated public static readonly string IMAGE_SGBRG8 = "image_sgbrg8";

        /// <summary>
        /// The SGRBG8.
        /// </summary>
        public static readonly string SGRBG8 = "image_sgrbg8";
        //// @deprecated public static readonly string IMAGE_SGRBG8 = "image_sgrbg8";

        /// <summary>
        /// The SRGGB8.
        /// </summary>
        public static readonly string SRGGB8 = "image_srggb8";
        //// @deprecated public static readonly string IMAGE_SRGGB8 = "image_srggb8";

        /// <summary>
        /// The SBGGR10.
        /// </summary>
        public static readonly string SBGGR10 = "image_sbggr10";
        //// @deprecated public static readonly string IMAGE_SBGGR10 = "image_sbggr10";

        /// <summary>
        /// The SGBRG10.
        /// </summary>
        public static readonly string SGBRG10 = "image_sgbrg10";
        //// @deprecated public static readonly string IMAGE_SGBRG10 = "image_sgbrg10";

        /// <summary>
        /// The SBRBG10.
        /// </summary>
        public static readonly string SGRBG10 = "image_sgrbg10";
        //// @deprecated public static readonly string IMAGE_SGRBG10 = "image_sgrbg10";

        /// <summary>
        /// The SRGGB10.
        /// </summary>
        public static readonly string SRGGB10 = "image_srggb10";
        //// @deprecated public static readonly string IMAGE_SRGGB10 = "image_srggb10";

        /// <summary>
        /// The SBGGR12.
        /// </summary>
        public static readonly string SBGGR12 = "image_sbggr12";
        //// @deprecated public static readonly string IMAGE_SBGGR12 = "image_sbggr12";

        /// <summary>
        /// The SGBRG12.
        /// </summary>
        public static readonly string SGBRG12 = "image_sgbrg12";
        //// @deprecated public static readonly string IMAGE_SGBRG12 = "image_sgbrg12";

        /// <summary>
        /// The SBRBG12.
        /// </summary>
        public static readonly string SGRBG12 = "image_sgrbg12";
        //// @deprecated public static readonly string IMAGE_SGRBG12 = "image_sgrbg12";

        /// <summary>
        /// The SRGGB12.
        /// </summary>
        public static readonly string SRGGB12 = "image_srggb12";
        //// @deprecated public static readonly string IMAGE_SRGGB12 = "image_srggb12";

        #endregion

        #region QuadBayer

        /// <summary>
        /// The quad SBGGR8.
        /// </summary>
        public static readonly string QuadSBGGR8 = "image_quad_sbggr8";

        /// <summary>
        /// The quad SGBRG8.
        /// </summary>
        public static readonly string QuadSGBRG8 = "image_quad_sgbrg8";

        /// <summary>
        /// The quad SGRBG8.
        /// </summary>
        public static readonly string QuadSGRBG8 = "image_quad_sgrbg8";

        /// <summary>
        /// The quad SRGGB8.
        /// </summary>
        public static readonly string QuadSRGGB8 = "image_quad_srggb8";

        /// <summary>
        /// The quad SBGGR10.
        /// </summary>
        public static readonly string QuadSBGGR10 = "image_quad_sbggr10";

        /// <summary>
        /// The quad SGBRG10.
        /// </summary>
        public static readonly string QuadSGBRG10 = "image_quad_sgbrg10";

        /// <summary>
        /// The quad SGRBG10.
        /// </summary>
        public static readonly string QuadSGRBG10 = "image_quad_sgrbg10";

        /// <summary>
        /// The quad SRGGB10.
        /// </summary>
        public static readonly string QuadSRGGB10 = "image_quad_srggb10";

        /// <summary>
        /// The quad SBGGR12.
        /// </summary>
        public static readonly string QuadSBGGR12 = "image_quad_sbggr12";

        /// <summary>
        /// The quad SGBRG12.
        /// </summary>
        public static readonly string QuadSGBRG12 = "image_quad_sgbrg12";

        /// <summary>
        /// The quad SGRBG12.
        /// </summary>
        public static readonly string QuadSGRBG12 = "image_quad_sgrbg12";

        /// <summary>
        /// The quad SRGGB12.
        /// </summary>
        public static readonly string QuadSRGGB12 = "image_quad_srggb12";

        #endregion

        #region Polarization image

        /// <summary>
        /// The polarization Y8 image.
        /// </summary>
        public static readonly string Polar_90_45_135_0_Y8 = "image_polar_90_45_135_0_y8";

        /// <summary>
        /// The polarization Y10 image.
        /// </summary>
        public static readonly string Polar_90_45_135_0_Y10 = "image_polar_90_45_135_0_y10";

        /// <summary>
        /// The polarization Y12 image.
        /// </summary>
        public static readonly string Polar_90_45_135_0_Y12 = "image_polar_90_45_135_0_y12";

        /// <summary>
        /// The polarization RGGB8 image.
        /// </summary>
        public static readonly string Polar_90_45_135_0_RGGB8 = "image_polar_90_45_135_0_rggb8";

        /// <summary>
        /// The polarization RGGB10 image.
        /// </summary>
        public static readonly string Polar_90_45_135_0_RGGB10 = "image_polar_90_45_135_0_rggb10";

        /// <summary>
        /// The polarization RGGB12 image.
        /// </summary>
        public static readonly string Polar_90_45_135_0_RGGB12 = "image_polar_90_45_135_0_rggb12";

        #endregion

        #region Compressed image

        /// <summary>
        /// The JPEG.
        /// </summary>
        public static readonly string JPEG = "image_jpeg";
        //// @deprecated public static readonly string IMAGE_JPEG = "image_jpeg";

        /// <summary>
        /// The H264.
        /// </summary>
        public static readonly string H264 = "image_h264";
        //// @deprecated public static readonly string IMAGE_H264 = "image_h264";

        #endregion

        #region Depth

        /// <summary>
        /// The 16-bit Z-Depth.
        /// </summary>
        public static readonly string Z16 = "depth_z16";
        //// @deprecated public static readonly string DEPTH_Z16 = "depth_z16";

        /// <summary>
        /// The 32-bit Z-Depth.
        /// </summary>
        public static readonly string Z32F = "depth_z32f";
        //// @deprecated public static readonly string DEPTH_Z32F = "depth_z32f";

        /// <summary>
        /// The 16-bit Disparity.
        /// </summary>
        public static readonly string D16 = "depth_d16";
        //// @deprecated public static readonly string DEPTH_D16 = "depth_d16";

        #endregion

        #region Confidence

        /// <summary>
        /// The 1-bit positive confidence.
        /// </summary>
        public static readonly string C1P = "confidence_c1p";
        //// @deprecated public static readonly string CONFIDENCE_C1P = "confidence_c1p";

        /// <summary>
        /// The 1-bit negative confidence.
        /// </summary>
        public static readonly string C1N = "confidence_c1n";
        //// @deprecated public static readonly string CONFIDENCE_C1N = "confidence_c1n";

        /// <summary>
        /// The 16-bit confidence.
        /// </summary>
        public static readonly string C16 = "confidence_c16";
        //// @deprecated public static readonly string CONFIDENCE_C16 = "confidence_c16";

        /// <summary>
        /// The 32-bit float confidence.
        /// </summary>
        public static readonly string C32F = "confidence_c32f";
        //// @deprecated public static readonly string CONFIDENCE_C32F = "confidence_c32f";

        #endregion

        #region PointCloud

        /// <summary>
        /// The signed 16-bit (x, y, depth).
        /// </summary>
        public static readonly string XYZ16 = "point_cloud_xyz16";
        //// @deprecated public static readonly string POINT_CLOUD_XYZ16 = "point_cloud_xyz16";

        /// <summary>
        /// The signed 16-bit (x, y, depth, rgb).
        /// </summary>
        public static readonly string XYZRGB16 = "point_cloud_xyzrgb16";
        //// @deprecated public static readonly string POINT_CLOUD_XYZRGB16 = "point_cloud_xyzrgb16";

        /// <summary>
        /// The signed 32-bit (x, y, depth).
        /// </summary>
        public static readonly string XYZ32 = "point_cloud_xyz32";
        //// @deprecated public static readonly string POINT_CLOUD_XYZ32 = "point_cloud_xyz32";

        /// <summary>
        /// The signed 32-bit (x, y, depth, rgb).
        /// </summary>
        public static readonly string XYZRGB32 = "point_cloud_xyzrgb32";
        //// @deprecated public static readonly string POINT_CLOUD_XYZRGB32 = "point_cloud_xyzrgb32";

        /// <summary>
        /// The unsigned 32-bit (x, y, depth).
        /// </summary>
        public static readonly string XYZ32U = "point_cloud_xyz32u";
        ///// @deprecated public static readonly string POINT_CLOUD_XYZ32U = "point_cloud_xyz32u";

        /// <summary>
        /// The unsigned 32-bit (x, y, depth, rgb).
        /// </summary>
        public static readonly string XYZRGB32U = "point_cloud_xyzrgb32u";
        //// @deprecated public static readonly string POINT_CLOUD_XYZRGB32U = "point_cloud_xyzrgb32u";

        /// <summary>
        /// The unsigned 16-bit (x, y, depth).
        /// </summary>
        public static readonly string XYZ16U = "point_cloud_xyz16u";
        //// @deprecated public static readonly string POINT_CLOUD_XYZ16U = "point_cloud_xyz16u";

        /// <summary>
        /// The unsigned 16-bit (x, y, depth, rgb).
        /// </summary>
        public static readonly string XYZRGB16U = "point_cloud_xyzrgb16u";
        //// @deprecated public static readonly string POINT_CLOUD_XYZRGB16U = "point_cloud_xyzrgb16u";

        /// <summary>
        /// The signed 32-bit float (x, y, depth).
        /// </summary>
        public static readonly string XYZ32F = "point_cloud_xyz32f";
        //// @deprecated public static readonly string POINT_CLOUD_XYZ32F = "point_cloud_xyz32f";

        /// <summary>
        /// The signed 32-bit float (x, y, depth, rgb).
        /// </summary>
        public static readonly string XYZRGB32F = "point_cloud_xyzrgb32f";
        //// @deprecated public static readonly string POINT_CLOUD_XYZRGB32F = "point_cloud_xyzrgb32f";

        /// <summary>
        /// The signed 16-bit (x, y, depth) planar array.
        /// </summary>
        public static readonly string XYZ16Planar = "point_cloud_xyz16_planar";
        //// @deprecated public static readonly string POINT_CLOUD_XYZ16Planar = "point_cloud_xyz16_planar";

        /// <summary>
        /// The unsigned 16-bit (x, y, depth) planar array.
        /// </summary>
        public static readonly string XYZ16UPlanar = "point_cloud_xyz16u_planar";
        //// @deprecated public static readonly string POINT_CLOUD_XYZ16UPlanar = "point_cloud_xyz16u_planar";

        /// <summary>
        /// The signed 32-bit float(x, y, depth) planar array.
        /// </summary>
        public static readonly string XYZ32FPlanar = "point_cloud_xyz32f_planar";
        //// @deprecated public static readonly string POINT_CLOUD_XYZ32FPlanar = "point_cloud_xyz32f_planar";

        #endregion

        #region GridMap

        /// <summary>
        /// The 1-bit positive voxel data.
        /// </summary>
        public static readonly string GRID_MAP_1P1N = "grid_map_1p1n";

        #endregion
    }
}
