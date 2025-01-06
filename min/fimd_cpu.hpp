/**
 * \file fimd_cpu.hpp
 * \author Vojtech Vrba (vrba.vojtech [at] fel.cvut.cz)
 * \date January 2025
 * \brief Compile-time Bresenham's circle generation and FIMD-CPU implementation (requires C++20).
 * \copyright GNU Public License.
 */

#ifndef FIMD_CPU_HPP
#define FIMD_CPU_HPP

#include <iostream>
#include <type_traits>
#include <array>
#include <cstdlib>

#include <list>

namespace fimd {

/**
 * \brief Alias for 2D points on an integer grid.
 */
using Point2D = std::array<int, 2>;


/**
 * \brief Computes the integer square root of a number at compile-time.
 * \param x The number to compute the square root of.
 * \param lo The lower bound of the search range.
 * \param hi The upper bound of the search range.
 * \return The integer square root of x.
 */
static consteval unsigned sqrt_recursive(const unsigned x, const unsigned lo, const unsigned hi) {
    return (lo == hi) ? lo : ((x / ((lo + hi + 1) / 2) < ((lo + hi + 1) / 2)) ? sqrt_recursive(x, lo, ((lo + hi + 1) / 2) - 1) : sqrt_recursive(x, ((lo + hi + 1) / 2), hi));
}


/**
 * \brief Compile-time estimation of the Bresenham boundary length.
 * \tparam R Radius of the circle.
 */
template<int R> requires (R > 0)
constexpr unsigned BresenhamBoundaryLengthEstimation = (R == 2) ? 4 : sqrt_recursive(2 * R * R, 0, R * R + 1) + 1;


/**
 * \brief Represents a boundary point in the first quadrant of a Bresenham's circle.
 * \tparam R Radius of the circle.
 * \tparam I Index of the point.
 * \tparam S Sequential order index.
 * \tparam X X-coordinate of the point.
 * \tparam Y Y-coordinate of the point.
 * \tparam P Decision parameter.
 * \tparam O Octant of the point.
 */
template<unsigned R, unsigned I, unsigned S, int X, int Y, int P, unsigned O>
class BresenhamBoundaryPoint : public Point2D {
private:
    static constexpr unsigned NextS = (O == 0) ? (BresenhamBoundaryLengthEstimation<R> - S - 1) : (BresenhamBoundaryLengthEstimation<R> - S);
    static constexpr Point2D XY = (O == 0) ? Point2D{X, Y} : Point2D{Y, X};

    using NextBresenhamBoundaryPoint = const std::conditional_t<(O == 1),
        std::conditional_t<(P < 0),
            BresenhamBoundaryPoint<R, I+1, NextS, X+1, Y, P+4*(X+1)+6, 0>,
            BresenhamBoundaryPoint<R, I+1, NextS, X+1, Y-1, P+4*((X+1)-(Y-1))+10, 0>
        >,
        BresenhamBoundaryPoint<R, I+1, NextS, X, Y, P, 1>
    >;
    static constexpr auto next = NextBresenhamBoundaryPoint();
public:
    constexpr BresenhamBoundaryPoint() : Point2D(XY) {};
    constexpr NextBresenhamBoundaryPoint get_next() { return next; };
};


/**
 * \brief Specialization of BresenhamBoundaryPoint for termination condition.
 * \tparam R Radius of the circle.
 * \tparam I Index of the point.
 * \tparam S Sequential order index.
 * \tparam X X-coordinate of the point.
 * \tparam Y Y-coordinate of the point.
 * \tparam P Decision parameter.
 * \tparam O Octant of the point.
 */
template<unsigned R, unsigned I, unsigned S, int X, int Y, int P, unsigned O> requires (X > Y or (X == Y and O == 1))
class BresenhamBoundaryPoint<R, I, S, X, Y, P, O> : public Point2D {
public:
    constexpr BresenhamBoundaryPoint() : Point2D({-1, -1}) {};
    constexpr BresenhamBoundaryPoint get_next() { return *this; };
};


/**
 * \brief Counts the number of boundary points in a Bresenham's circle quadrant.
 * \tparam R Radius of the circle.
 * \tparam I Index of the point.
 * \tparam S Sequential order index.
 * \tparam X X-coordinate of the point.
 * \tparam Y Y-coordinate of the point.
 * \tparam P Decision parameter.
 * \tparam O Octant of the point.
 * \param point The initial boundary point.
 * \return The number of boundary points.
 */
template<unsigned R, unsigned I, unsigned S, int X, int Y, int P, unsigned O>
consteval unsigned BresenhamBoundaryCounter(BresenhamBoundaryPoint<R, I, S, X, Y, P, O> point) {
    if (point != point.get_next()) {
        return BresenhamBoundaryCounter(point.get_next());
    }
    return I;
}


/**
 * \brief Alias for the initial boundary point in Bresenham's circle algorithm.
 * \tparam R Radius of the circle.
 */
template<int R> requires (R > 0)
using BresenhamBoundaryInit = BresenhamBoundaryPoint<R, 0, 0, 0, R, 3-2*R, 0>;


/**
 * \brief Compile-time calculation of the Bresenham boundary length.
 * \tparam R Radius of the circle.
 */
template<int R> requires (R > 0)
constexpr unsigned BresenhamBoundaryLength = BresenhamBoundaryCounter(BresenhamBoundaryInit<R>());


/**
 * \brief Represents the boundary of a circle using Bresenham's algorithm.
 * \tparam R Radius of the circle.
 */
template<int R> requires (R > 0)
class BresenhamBoundary : public std::array<Point2D, 4*(BresenhamBoundaryLength<R>-1)>
{
public:
    consteval BresenhamBoundary() {
        seq_sort_recursive(*this, BresenhamBoundaryInit<R>());
        eval_sort(*this);
    };

    static std::ostream& print_verbose(std::ostream& os) {
        os << "Bresenham boundary for R=" << radius() << ": quadrant length = " << length() << std::endl;
        return print_recursive(os, BresenhamBoundaryInit<R>());
    }

    constexpr std::ostream& print_sorted(std::ostream& os) {
        os << "Sorted points (" << length() << "): [";
        for (auto point : *this) {
            os << "(" << point[0] << "," << point[1] << "), ";
        }
        os << "\b\b]" << std::endl;
        return os;
    }

    static constexpr unsigned length_quadrant() {
        return BresenhamBoundaryLength<R>;
    }

    static constexpr unsigned length() {
        return 4 * (length_quadrant() - 1);
    }

    static constexpr unsigned length_quadrant_estimation() {
        return BresenhamBoundaryLengthEstimation<R>;
    }

    static constexpr int radius() {
        return R;
    }

private:
    template<unsigned R2, unsigned I, unsigned S, int X, int Y, int P, unsigned O>
    static constexpr std::ostream& print_recursive(std::ostream& os, BresenhamBoundaryPoint<R2, I, S, X, Y, P, O> point) {
        if (point != point.get_next()) {
            os << "-- Point #" << (I+1) << ": (" << point[0] << ", " << point[1] << ") [S=" << S << ", X=" << X << ", Y=" << Y << ", P=" << P << ", O=" << O << "]" << std::endl;
            return print_recursive(os, point.get_next());
        }
        return os;
    };

    template<unsigned R2, unsigned I, unsigned S, int X, int Y, int P, unsigned O>
    static consteval void seq_sort_recursive(std::array<Point2D, length()> &points, BresenhamBoundaryPoint<R2, I, S, X, Y, P, O> point) {
        if (point != point.get_next()) {
            constexpr unsigned index = (O == 1 and length_quadrant_estimation() > length_quadrant()) ? S - 1 : S;
            if (index < length_quadrant() - 1) {
                points[index] = {point[0], point[1]};
                points[index + (length_quadrant() - 1)] = {point[1], -point[0]};
                points[index + 2 * (length_quadrant() - 1)] = {-point[0], -point[1]};
                points[index + 3 * (length_quadrant() - 1)] = {-point[1], point[0]};
            }
            seq_sort_recursive(points, point.get_next());
        }
    };

    static consteval void eval_sort(std::array<Point2D, length()> &points) {
        auto eval = std::array<Point2D, length()>();
        auto dists = std::array<int, length_quadrant()>();
        for (int i = 0; i < length_quadrant(); i++) {
            dists[i] = std::max(R - points[i][0], points[i][1]);
        }

        int i_next = 0;
        unsigned cnt = 0;
        while (cnt < length()) {
            auto point = points[i_next];
            eval[cnt++] = {point[0], point[1]};
            eval[cnt++] = {-point[0], -point[1]};
            if (point[0] == 0) {
                eval[cnt++] = {point[1], 0};
                eval[cnt++] = {-point[1], 0};
            } else {
                eval[cnt++] = {-point[1], point[0]};
                eval[cnt++] = {point[1], -point[0]};
            }
            dists[i_next] = 0;

            int sel_d = 0;
            int sel_y = 0;
            int sel_i = 0;
            for (int i = 0; i < length_quadrant(); i++) {
                auto point2 = points[i];
                auto dist = std::max(std::abs(point2[0] - point[0]), std::abs(point2[1] - point[1]));
                if (dists[i] > dist) {
                    dists[i] = dist;
                }
                if (dists[i] > sel_d or (dists[i] == sel_d and point2[1] > sel_y)) {
                    sel_d = dists[i];
                    sel_y = point2[1];
                    sel_i = i;
                }
            }
            i_next = sel_i;
        }

        for (int i = 0; i < length(); i++) {
            points[i] = eval[i];
        }
    };
};


/**
 * \brief Represents an interior point in upper half of a Bresenham's circle.
 * \tparam I Index of the point.
 * \tparam X X-coordinate of the point.
 * \tparam Y Y-coordinate of the point.
 * \tparam P Decision parameter.
 * \tparam O Octant of the point.
 * \tparam YI Y-coordinate increment.
 */
template<unsigned I, int X, int Y, int P, unsigned O, int YI>
class BresenhamInteriorPoint : public Point2D {
    using NextBresenhamInteriorPoint = const std::conditional_t<(YI < Y-1),
        BresenhamInteriorPoint<I+1, X, Y, P, O, YI+1>,
        std::conditional_t<(O == 0),
            std::conditional_t<(X < YI) or (X == 0),
                BresenhamInteriorPoint<I+1, X, Y, P, 1, X+1>,
                BresenhamInteriorPoint<I+1, X, Y, P, 2, X>
            >,
            std::conditional_t<(O == 1) and (X > 0),
                BresenhamInteriorPoint<I+1, X, Y, P, 2, X>,
                std::conditional_t<(O == 2),
                    BresenhamInteriorPoint<I+1, X, Y, P, 3, X+1>,
                    std::conditional_t<(P < 0),
                        BresenhamInteriorPoint<I+1, X+1, Y, P+4*(X+1)+6, 0, X+1>,
                        BresenhamInteriorPoint<I+1, X+1, Y-1, P+4*((X+1)-(Y-1))+10, 0, X+1>
                    >
                >
            >
        >
    >;
    using XY = const std::conditional_t<(O == 0),
        std::integral_constant<Point2D, {X, YI}>,
        std::conditional_t<(O == 1),
            std::integral_constant<Point2D, {YI, X}>,
            std::conditional_t<(O == 2),
                std::integral_constant<Point2D, {-X, YI}>,
                std::integral_constant<Point2D, {-YI, X}>
            >
        >
    >;

    static constexpr auto next = NextBresenhamInteriorPoint();

public:
    constexpr BresenhamInteriorPoint() : Point2D(XY::value) {};
    constexpr NextBresenhamInteriorPoint get_next() { return next; };
};


/**
 * \brief Specialization of BresenhamInteriorPoint for termination condition.
 * \tparam I Index of the point.
 * \tparam X X-coordinate of the point.
 * \tparam Y Y-coordinate of the point.
 * \tparam P Decision parameter.
 * \tparam O Octant of the point.
 * \tparam YI Interior coordinate increment.
 */
template<unsigned I, int X, int Y, int P, unsigned O, int YI> requires (YI >= Y)
class BresenhamInteriorPoint<I, X, Y, P, O, YI> : public Point2D {
public:
    constexpr BresenhamInteriorPoint() : Point2D({-1, -1}) {};
    constexpr BresenhamInteriorPoint get_next() { return *this; };
};


/**
 * \brief Counts the number of interior points in a Bresenham's circle.
 * \tparam I Index of the point.
 * \tparam X X-coordinate of the point.
 * \tparam Y Y-coordinate of the point.
 * \tparam P Decision parameter.
 * \tparam O Octant of the point.
 * \tparam YI Interior coordinate increment.
 * \param point The current interior point.
 * \return The number of interior points.
 */
template<unsigned I, int X, int Y, int P, unsigned O, int YI>
consteval unsigned BresenhamInteriorCounter(BresenhamInteriorPoint<I, X, Y, P, O, YI> point) {
    if (point != point.get_next()) {
        return BresenhamInteriorCounter(point.get_next());
    }
    return I;
}


/**
 * \brief Alias for the initial interior point in Bresenham's circle algorithm.
 * \tparam R Radius of the circle.
 */
template<int R> requires (R > 0)
using BresenhamInteriorInit = BresenhamInteriorPoint<0, 0, R, 3-2*R, 0, 0>;


/**
 * \brief Compile-time calculation of the Bresenham interior length.
 * \tparam R Radius of the circle.
 */
template<int R> requires (R > 0)
constexpr unsigned BresenhamInteriorLength = BresenhamInteriorCounter(BresenhamInteriorInit<R>());


/**
 * \brief Represents the interior of a circle using Bresenham's algorithm.
 * \tparam R Radius of the circle.
 */
template<int R> requires (R > 0)
class BresenhamInterior : public std::array<Point2D, BresenhamInteriorLength<R>>
{
public:
    consteval BresenhamInterior() {
        copy_recursive(*this, BresenhamInteriorInit<R>());
    };

    static std::ostream& print_verbose(std::ostream& os) {
        os << "Bresenham interior for R=" << radius() << ": half length = " << length() << std::endl;
        return print_recursive(os, BresenhamInteriorInit<R>());
    }

    static constexpr unsigned length() {
        return BresenhamInteriorLength<R>;
    }

    static constexpr int radius() {
        return R;
    }

private:
    template<unsigned I, int X, int Y, int P, unsigned O, int YI>
    static constexpr std::ostream& print_recursive(std::ostream& os, BresenhamInteriorPoint<I, X, Y, P, O, YI> point) {
        if (point != point.get_next()) {
            os << "-- Point #" << (I+1) << ": (" << point[0] << ", " << point[1] << ") [X=" << X << ", Y=" << Y << ", P=" << P << ", O=" << O << ", YI=" << YI << "]" << std::endl;
            return print_recursive(os, point.get_next());
        }
        return os;
    };

    template<unsigned I, int X, int Y, int P, unsigned O, int YI>
    static consteval void copy_recursive(std::array<Point2D, length()> &points, BresenhamInteriorPoint<I, X, Y, P, O, YI> point) {
        if (point != point.get_next()) {
            points[I] = {point[0], point[1]};
            copy_recursive(points, point.get_next());
        }
    };
};


/**
 * \brief Fast Isolated Marker Detection (FIMD) CPU implementation.
 * \tparam RADIUS Radius of the circle.
 */
template<unsigned RADIUS>
class FIMD_CPU {
public:
    /**
     * \brief FIMD-CPU constructor.
     * \param im_width Image width.
     * \param im_height Image height.
     * \param threshold_center Center threshold.
     * \param threshold_diff Difference threshold.
     * \param threshold_sun Sun threshold.
     * \param termination Termination sequence (2 bytes).
     * \param max_markers_count Limit for the number of the detected markers (0 for no limit).
     * \param max_sun_points_count Limit for the number of the detected sun points (0 for no limit).
     */
    FIMD_CPU(unsigned im_width, unsigned im_height, unsigned char threshold_center=120, unsigned char threshold_diff=60, unsigned char threshold_sun=240, unsigned short termination=0x00FF, unsigned max_markers_count=0, unsigned max_sun_points_count=0)
    : im_width_(im_width), im_height_(im_height), threshold_center_(threshold_center), threshold_diff_(threshold_diff), threshold_sun_(threshold_sun), termination_(termination)
    {
        offset_ = (im_width_ * RADIUS) + RADIUS;
        max_markers_count_ = (max_markers_count == 0) ? im_width_ * im_height_ : max_markers_count;
        max_sun_points_count_ = (max_sun_points_count == 0) ? im_width_ * im_height_ : max_sun_points_count;
        frame_ = static_cast<unsigned char*>(std::malloc(im_width_ * im_height_ * sizeof(unsigned char)));
    };

    ~FIMD_CPU() {
        std::free(frame_);
    };

    /**
     * \brief Converts a 2D point to a 1D coordinate.
     * \param point The 2D point.
     * \return The 1D coordinate.
     */
    [[nodiscard]] int coord1d(Point2D point) const { return (point[1] * static_cast<int>(im_width_)) + point[0]; };

    /**
     * \brief Detects markers and sun points in an image.
     * \param image The image data (array of unsigned bytes).
     * \param markers The list of detected markers.
     * \param sun_points The list of detected sun points.
     * \param make_copy If true, a copy of the input image will be used in the detection process.
     * \return The total number of processed pixels in the input image.
     */
    unsigned detect(unsigned char* image, std::list<Point2D> &markers, std::list<Point2D> &sun_points, bool make_copy=true) {
        unsigned char* target_image;
        if (make_copy) {
            std::copy_n(image, im_height_ * im_width_, frame_);
            target_image = frame_;
        } else {
            target_image = image;
        }

        *reinterpret_cast<unsigned short*>((target_image) + (im_width_ * im_height_) - 2) = termination_;
        unsigned char* cursor = target_image + offset_ - 1;

        int sun_point_x, sun_point_y;
        int marker_x, marker_y;

        LOOP:
            // check for the presence of the termination sequence
            if (*reinterpret_cast<unsigned short*>((cursor) + offset_) == termination_) {
                return (reinterpret_cast<size_t>(cursor) - reinterpret_cast<size_t>(target_image));
            }

            // load new pixel value from pre-incremented address
            unsigned char pix_val = *(++cursor);
            if (pix_val <= threshold_center_) goto LOOP;

            // first boundary pixel test - decide between MARKER_TEST and SUN_TEST
            if ((pix_val - *(cursor + coord1d(boundary[0]))) <= threshold_diff_) {
                if (pix_val >= threshold_sun_) goto SUN_TEST;
            } else {
                goto MARKER_TEST;
            }

            // otherwise go to the next pixel
            goto LOOP;

        SUN_TEST:
            // testing for boundary pixels
            if (boundary_unroll([&](const Point2D point) -> bool {
                return (pix_val - *(cursor + coord1d(point))) > threshold_diff_;
            })) goto LOOP;

            // clearing interior pixels
            interior_unroll([&](const Point2D point) -> bool {
                *(cursor + coord1d(point)) = 0x00;
                return false;
            });

            sun_point_x = (reinterpret_cast<size_t>(cursor) - reinterpret_cast<size_t>(target_image)) % im_width_;
            sun_point_y = (reinterpret_cast<size_t>(cursor) - reinterpret_cast<size_t>(target_image)) / im_width_;
            sun_points.push_back({sun_point_x, sun_point_y});

            // check the current number of the detected sun points
            if (sun_points.size() == max_sun_points_count_) {
                *reinterpret_cast<unsigned short*>((cursor) + offset_) = termination_;
            }
            goto LOOP;

        MARKER_TEST:
            // testing for boundary pixels
            if (boundary_unroll([&](const Point2D point) -> bool {
                return (pix_val - *(cursor + coord1d(point))) <= threshold_diff_;
            })) goto LOOP;

            // search for peak in interior
            unsigned char peak = 0;
            size_t peak_pos = 0;
            unsigned char* curr_int_ptr = nullptr;

            interior_unroll([&](const Point2D point) -> bool {
                curr_int_ptr = cursor + coord1d(point);
                if (*curr_int_ptr > peak) {
                    peak = *curr_int_ptr;
                    peak_pos = reinterpret_cast<size_t>(curr_int_ptr) - reinterpret_cast<size_t>(target_image);
                }
                *curr_int_ptr = 0x00;
                return false;
            });

            marker_x = peak_pos % im_width_;
            marker_y = peak_pos / im_width_;
            markers.push_back({marker_x, marker_y});

            // check the current number of the detected markers
            if (markers.size() == max_markers_count_) {
                *reinterpret_cast<unsigned short*>((cursor) + offset_) = termination_;
            }
            goto LOOP;

        return 0;
    };

private:
    unsigned im_width_;
    unsigned im_height_;
    unsigned offset_;
    unsigned char threshold_center_;
    unsigned char threshold_diff_;
    unsigned char threshold_sun_;
    unsigned short termination_;
    unsigned max_markers_count_;
    unsigned max_sun_points_count_;
    unsigned char* frame_;

    static constexpr auto boundary = BresenhamBoundary<RADIUS>();
    static constexpr auto interior = BresenhamInterior<RADIUS>();

    template<unsigned... I, class F>
    static constexpr bool boundary_unroll_helper(std::integer_sequence<unsigned, I...>, F&& f) {
        return (f(boundary[I]) or ...);
    }

    template<class F>
    static constexpr bool boundary_unroll(F&& f) {
        return boundary_unroll_helper(std::make_integer_sequence<unsigned, boundary.length()>{}, std::forward<F>(f));
    }

    template<unsigned... I, class F>
    static constexpr bool interior_unroll_helper(std::integer_sequence<unsigned, I...>, F&& f) {
        return (f(interior[I]) or ...);
    }

    template<class F>
    static constexpr bool interior_unroll(F&& f) {
        return interior_unroll_helper(std::make_integer_sequence<unsigned, interior.length()>{}, std::forward<F>(f));
    }
};

};

#endif // FIMD_CPU_HPP
