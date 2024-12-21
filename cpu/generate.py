# -*- coding: utf-8 -*-
"""
Created on Fri Dec 20 12:42:07 2024
@author: Vojtech Vrba (vrba.vojtech@fel.cvut.cz)

Script for generation of FIMD-CPU approach using templates.
"""

from argparse import ArgumentParser
from os import path
from sys import exit, argv, stdout


def print_circle(boundary: list, interior: list):
    """
    Prints the boundary and interior pixels in ASCII format.

    Args:
        boundary (list of tuples): Coordinates of boundary pixels.
        interior (list of tuples): Coordinates of interior pixels.
    """
    y_max, y_min = max([y for y, x in boundary]), min([y for y, x in boundary])
    x_max, x_min = max([y for y, x in boundary]), min([y for y, x in boundary])
    radius = max(y_max - y_min, x_max - x_min) // 2
    n_pts_max = max(len(boundary), len(interior))
    digits = 0
    while n_pts_max > 0:
        n_pts_max //= 10
        digits += 1
    
    grid = [[" " * (digits+1) for x in range(2*radius + 1)] for y in range(2*radius + 1)]
    
    for i, (y, x) in enumerate(boundary):
        if len(grid[y-y_min][x-x_min].strip()) == 0:
            grid[y-y_min][x-x_min] = "B%0*d" % (digits, i + 1)
        else:
            grid[y-y_min][x-x_min] = "?" * (digits + 1)
    
    for i, (y, x) in enumerate(interior):
        if len(grid[y-y_min][x+radius].strip()) == 0:
            grid[y-y_min][x-x_min] = "I%0*d" % (digits, i + 1)
        else:
            grid[y-y_min][x-x_min] = "?" * (digits + 1)
    
    for row in grid:
        print(*row)


def bresenham_circle_points(r: int, x0: int = 0, y0 : int = 0) -> tuple:
    """
    Generates boundary and interior points of a circle using Bresenham's algorithm.

    Args:
        r (int): Radius of the circle.
        x0 (int, optional): X-coordinate of the circle center. Defaults to 0.
        y0 (int, optional): Y-coordinate of the circle center. Defaults to 0.

    Returns:
        tuple: Two lists containing boundary and interior points in the generation order.
    """
    boundary = list()
    interior = list()
    x, y = (0, r)
    P = 3 - 2*r
    
    while x <= y:            
        boundary.append((y0+y, x0-x))
        if r == 0: break
        boundary.append((y0-y, x0+x))
        if x < y: # two more points not on the diagonal y=x
            boundary.append((y0+x, x0-y))
            boundary.append((y0-x, x0+y))
        
        if x > 0: # points not on the y-axis
            boundary.append((y0+y, x0+x))
            boundary.append((y0-y, x0-x))
            if x < y: # two more points not on the diagonal y=x
                boundary.append((y0+x, x0+y))
                boundary.append((y0-x, x0-y))
        
        for y_i in range(x, y):
            interior.append((y0+y_i, x0+x))
            if y_i > x:
                interior.append((x0+x, y0+y_i))
                interior.append((x0+x, y0-y_i))
            if y_i > 0:
                interior.append((y0-y_i, x0+x))
            if x > 0:
                interior.append((y0+y_i, x0-x))
                interior.append((y0-y_i, x0-x))
                if y_i > x:
                    interior.append((x0-x, y0+y_i))
                    interior.append((x0-x, y0-y_i))
                    
        if P < 0:
            x += 1
            P += 4*x + 6
        else:
            x += 1
            y -= 1
            P += 4*(x-y) + 10
    
    return boundary, interior


def get_boundary_evaluation_order(boundary: list) -> list:
    """
    Sorts boundary pixels to determine the evaluation order for FIMD.
    Picks the most mutually distant points from a single quadrant.
    In case of multiple equally distant points, point with the largest y coordinate is chosen.
    Evaluation order starts with the first point from the generation order, i.e., (0, r).

    Args:
        boundary (list of tuples): Coordinates of boundary pixels.

    Returns:
        list: Sorted list of boundary pixel coordinates in evaluation order.
    """
    quadrant = [(y, x) for y, x in boundary if y >= 0 and x >= 0]
    radius = max([y for y, x in boundary])
    evaluation_order = list()
    quadrant_dists = [max(y, radius-x) for y, x in quadrant]
    i_next = 0
    
    while len(evaluation_order) < len(boundary):
        y, x = quadrant[i_next]
        evaluation_order.append((y, x))
        evaluation_order.append((-y, -x))
        if x == 0:
            evaluation_order.append((0, y))
            evaluation_order.append((0, -y))
        else:
            evaluation_order.append((x, -y))
            evaluation_order.append((-x, y))
        quadrant_dists[i_next] = 0
        
        sel_d_y = (0, 0)
        sel_i = 0
        for i in range(len(quadrant_dists)):
            y_i, x_i = quadrant[i]
            dist = max(abs(y_i - y), abs(x_i - x))
            if quadrant_dists[i] > dist:
                quadrant_dists[i] = dist
                
            if quadrant_dists[i] > sel_d_y[0] or (quadrant_dists[i] == sel_d_y[0] and y_i > sel_d_y[1]):
                sel_d_y = (quadrant_dists[i], y_i)
                sel_i = i
        i_next = sel_i
    
    return evaluation_order



if __name__ == "__main__":
    parser = ArgumentParser(description="Script for generation of FIMD-CPU approach using templates.")
    parser.add_argument("-r", "--radius", type=int, required=True, help="Radius of the circle to generate.")
    parser.add_argument("-t", "--template", type=str, required=True, help="Template file for the code generation.")
    parser.add_argument("-o", "--output", type=str, required=True, help="Output file for the generated code.")
    parser.add_argument("-v", "--verbose", action="store_true", help="Prints the generated code to the console.")

    if len(argv) == 1:
        parser.print_help(stdout)
        exit(0)

    args = parser.parse_args()

    if not path.exists(args.template):
        print("Error: Template file '%s' not found." % args.template)
        exit(1)

    if args.verbose:
        print("Starting", parser.description)
        print("Selected circle radius:", args.radius)
    
    boundary, interior = bresenham_circle_points(args.radius)

    if args.verbose:
        print("\nGenerated Bresenham circle:")
        print("-- Boundary points:", len(boundary))
        print("-- Interior points:", len(interior))
        print("Visualization:")
        print_circle(boundary, interior)

    FIMD_RADIUS = args.radius
    FIMD_BOUNDARY = get_boundary_evaluation_order(boundary)
    FIMD_INTERIOR = list(sorted([(y, x) for y, x in interior if y > 0 or (y == 0 and x >= 0)]))

    if args.verbose:
        print("\nPixel evaluation order:")
        print("-- Boundary points:", len(FIMD_BOUNDARY))
        print("-- Interior points:", len(FIMD_INTERIOR))
        print("Visualization:")
        print_circle(FIMD_BOUNDARY, FIMD_INTERIOR)

    if args.verbose:
        print("Generating code using template:", args.template)

    template = ""
    with open(args.template, "r") as f:
        for line in f.readlines():
            template += line.replace("//$ ", "")
    
    GEN_OUTPUT = list()
    code = compile(template, "<string>", "exec")
    exec(code)

    with open(args.output, "w") as f:
        f.write("".join(GEN_OUTPUT))


    if args.verbose:
        print("Written generated code to file:", args.output)
        print("Script finished.")
    else:
        print("[Python generator] Finished: Radius: %4d, Boundary length: %4d, Interior length: %4d | Written as: %s" % (FIMD_RADIUS, len(FIMD_BOUNDARY), len(FIMD_INTERIOR), args.output))

