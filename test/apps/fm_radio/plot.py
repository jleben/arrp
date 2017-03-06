import islpy as isl
import islplot.plotter3d
import islplot.plotter
import sys
from IPython.display import HTML
import matplotlib.pyplot as plt
from matplotlib import pylab

def main():

  ctx = isl.Context()

  a_s = isl.Map.read_from_str(ctx, "{ a_4_s[i0, i1, i2] -> [i0 + i2, i1, 10 + i2, 13] : i0 >= 0 and i1 >= 0 and i1 <= 9 and i2 >= 0 and i2 <= 63 and i0 < 20}")
  f1_s = isl.Map.read_from_str(ctx, "{ folding_4_s1[i0, i1, i2] -> [i0 + i2, 9 + i1, 18 + i2, 11] : i0 >= 0 and i1 >= 0 and i1 <= 9 and i2 >= 1 and i2 <= 63  and i0 < 20}")
  f0_s = isl.Map.read_from_str(ctx, "{ folding_4_s0[i0, i1, 0] -> [i0, 9, 10 + i1, 2] : i0 >= 0 and i1 >= 0 and i1 <= 9  and i0 < 20}")

  a_f0 = isl.Map.read_from_str(ctx, "{ a_4_s[i0, i1, 0] -> folding_4_s0[i0, i1, 0] : i0 >= 0 and i1 >= 0 and i1 <= 9 }")
  a_f1 = isl.Map.read_from_str(ctx, "{ a_4_s[i0, i1, i2] -> folding_4_s1[i0, i1, i2] : i0 >= 0 and i1 >= 0 and i1 <= 9 and i2 >= 1 and i2 <= 63 }")

  a_t = a_s.range()
  f1_t = f1_s.range()
  f0_t = f0_s.range()

  a_f0_t = a_f0.apply_domain(a_s).apply_range(f0_s)
  a_f1_t = a_f1.apply_domain(a_s).apply_range(f1_s)

  print a_t
  print f1_t
  print f0_t
  print a_f0_t
  #islplot.plotter.plot_set_points(a_t.project_out(isl.dim_type.set, 0, 1), 'blue')
  #islplot.plotter.plot_set_points(f0_t.project_out(isl.dim_type.set, 0, 1), 'red')
  #islplot.plotter.plot_set_points(f1_t.project_out(isl.dim_type.set, 0, 1), 'green')
  #islplot.plotter.plot_map(a_f0_t)
  islplot.plotter.plot_map(isl.Map("{ [i0, i1] -> [i0, 9] : i0 >= 0 and i1 >= 0 and i1 <= 9 and i0 <= 19 }"), edge_width=3)
  #islplot.plotter.plot_map(isl.Map.read_from_str(ctx, "{[2,8] -> [x,y]: 1 < x < 9 and 1 < y < 9 and x = y}"))
  #islplot.plotter.plot_map(a_f1_t)
  pylab.show()

main()
