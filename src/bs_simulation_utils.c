#include <bs_simulation_utils.h>

#include <math.h>

Tuple(double, double) get_gforce(BodyInfo *const b1, BodyInfo *const b2) {
  double const dx = (*global_ctx_body_access_member(b2, BI_POS_X) - *global_ctx_body_access_member(b1, BI_POS_X));
  double const dy = (*global_ctx_body_access_member(b2, BI_POS_Y) - *global_ctx_body_access_member(b1, BI_POS_Y));
  double const r2 = dx * dx + dy * dy;

  if(r2 == 0)
    return (Tuple(double, double)){0.0, 0.0};

  double const r = sqrt(r2);
  double const inv_r3 = 1.0 / (r * r * r);
  double const factor =
    G
    * (*global_ctx_body_access_member(b1, BI_MASS))
    * (*global_ctx_body_access_member(b2, BI_MASS))
    * inv_r3;

  return (Tuple(double, double)){factor * dx, factor * dy};
}