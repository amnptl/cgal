// Copyright (c) 2006  Tel-Aviv University (Israel).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org); you may redistribute it under
// the terms of the Q Public License version 1.0.
// See the file LICENSE.QPL distributed with CGAL.
//
// Licensees holding a valid commercial license may use this file in
// accordance with the commercial license agreement provided with the software.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// $URL$
// $Id$
// 
//
// Author(s)     : Ron Wein <wein@post.tau.ac.il>

#ifndef CGAL_ARR_UNB_PLANAR_TOPOLOGY_TRAITS_2_IMPL_H
#define CGAL_ARR_UNB_PLANAR_TOPOLOGY_TRAITS_2_IMPL_H

/*! \file
 * Member-function definitions for the
 * Arr_unb_planar_topology_traits_2<GeomTraits> class.
 */

CGAL_BEGIN_NAMESPACE

//-----------------------------------------------------------------------------
// Default constructor.
//
template <class GeomTraits, class Dcel_>
Arr_unb_planar_topology_traits_2<GeomTraits, Dcel_>::
Arr_unb_planar_topology_traits_2():
  Base (),
  v_bl (NULL),
  v_tl (NULL),
  v_br (NULL),
  v_tr (NULL),
  n_inf_verts (0),
  fict_face (NULL)
{}

//-----------------------------------------------------------------------------
// Constructor with a geometry-traits class.
//
template <class GeomTraits, class Dcel_>
Arr_unb_planar_topology_traits_2<GeomTraits, Dcel_>::
Arr_unb_planar_topology_traits_2 (Geometry_traits_2 *tr) :
  Base (tr),
  v_bl (NULL),
  v_tl (NULL),
  v_br (NULL),
  v_tr (NULL),
  n_inf_verts (0),
  fict_face (NULL)
{}

//-----------------------------------------------------------------------------
// Assign the contents of another topology-traits class.
//
template <class GeomTraits, class Dcel_>
void Arr_unb_planar_topology_traits_2<GeomTraits, Dcel_>::assign
    (const Self& other)
{
  // Assign the base class.
  Base::assign (other);

  // Update the topology-traits properties after the DCEL have been updated.
  dcel_updated();

  return;
}

//-----------------------------------------------------------------------------
// Make the necessary updates after the DCEL structure have been updated.
//
template <class GeomTraits, class Dcel_>
void Arr_unb_planar_topology_traits_2<GeomTraits, Dcel_>::dcel_updated ()
{
  // Go over the DCEL vertices and locate the four fictitious ones.
  typename Dcel::Vertex_iterator       vit;
  Boundary_type                        bx, by;
  Halfedge                            *first_he, *next_he;

  v_bl = v_tl = v_br = v_tr = NULL;
  n_inf_verts = 0;
  for (vit = this->m_dcel.vertices_begin();
       vit != this->m_dcel.vertices_end(); ++vit)
  {
    if (! vit->has_null_point())
      continue;

    n_inf_verts++;

    // The current vertex is not associated with a point - check if it has
    // only two incident halfedges. If so, it is one of the four fictitious
    // vertices.
    first_he = vit->halfedge();
    next_he = first_he->next()->opposite();

    if (next_he->next()->opposite() == first_he)
    {
      bx = vit->boundary_in_x();
      by = vit->boundary_in_y();

      if (bx == MINUS_INFINITY && by == MINUS_INFINITY)
        v_bl = &(*vit);
      else if (bx == MINUS_INFINITY && by == PLUS_INFINITY)
        v_tl = &(*vit);
      else if (bx == PLUS_INFINITY && by == MINUS_INFINITY)
        v_br = &(*vit);
      else if (bx == PLUS_INFINITY && by == PLUS_INFINITY)
        v_tr = &(*vit);
      else
        // We should never reach here:
        CGAL_assertion (false);
    }
  }
  CGAL_assertion (v_bl != NULL && v_tl != NULL &&
                  v_br != NULL && v_tr != NULL);

  // Go over the DCEL faces and locate the fictitious face.
  typename Dcel::Face_iterator         fit;
  
  fict_face = NULL;
  for (fit = this->m_dcel.faces_begin();
       fit != this->m_dcel.faces_end(); ++fit)
  {
    if (fit->is_fictitious())
    {
      CGAL_assertion (fict_face == NULL);

      fict_face = &(*fit);
    }
  }
  CGAL_assertion (fict_face != NULL);

  return;
}

//-----------------------------------------------------------------------------
// Initialize an empty DCEL structure.
//
template <class GeomTraits, class Dcel_>
void Arr_unb_planar_topology_traits_2<GeomTraits, Dcel_>::init_dcel ()
{
  // Clear the current DCEL.
  this->m_dcel.delete_all();

  // Create the fictitious unbounded face.
  fict_face = this->m_dcel.new_face();

  fict_face->set_unbounded (true);
  fict_face->set_fictitious (true);

  // Create the four fictitious vertices corresponding to corners of the
  // bounding rectangle.
  v_bl = this->m_dcel.new_vertex();
  v_bl->set_boundary (MINUS_INFINITY, MINUS_INFINITY);

  v_tl = this->m_dcel.new_vertex();
  v_tl->set_boundary (MINUS_INFINITY, PLUS_INFINITY);

  v_br = this->m_dcel.new_vertex();
  v_br->set_boundary (PLUS_INFINITY, MINUS_INFINITY);

  v_tr = this->m_dcel.new_vertex();
  v_tr->set_boundary (PLUS_INFINITY, PLUS_INFINITY);

  // Create a four pairs of twin halfedges connecting the two vertices,
  // and link them together to form the bounding rectangle, forming a hole
  // in the fictitious face.
  //
  //                            he2
  //             v_tl (.) ----------------> (.) v_tr
  //                   ^ <------------------ 
  //                   ||                   ^|
  //  fict_face    he1 ||        in_f       ||
  //                   ||                   || he3
  //                   |V                   ||
  //                     ------------------> V 
  //             v_bl (.) <---------------- (.) v_br
  //                             he4
  //
  Halfedge           *he1 = this->m_dcel.new_edge();
  Halfedge           *he1_t = he1->opposite();
  Halfedge           *he2 = this->m_dcel.new_edge();
  Halfedge           *he2_t = he2->opposite();
  Halfedge           *he3 = this->m_dcel.new_edge();
  Halfedge           *he3_t = he3->opposite();
  Halfedge           *he4 = this->m_dcel.new_edge();
  Halfedge           *he4_t = he4->opposite();
  Outer_ccb          *oc = this->m_dcel.new_outer_ccb();
  Inner_ccb          *ic = this->m_dcel.new_inner_ccb();
  Face               *in_f = this->m_dcel.new_face();

  he1->set_curve (NULL);
  he2->set_curve (NULL);
  he3->set_curve (NULL);
  he4->set_curve (NULL);

  he1->set_next (he2);        he1_t->set_next (he4_t);
  he2->set_next (he3);        he4_t->set_next (he3_t);
  he3->set_next (he4);        he3_t->set_next (he2_t);
  he4->set_next (he1);        he2_t->set_next (he1_t);

  he1->set_vertex (v_tl);     he1_t->set_vertex (v_bl);
  he2->set_vertex (v_tr);     he2_t->set_vertex (v_tl);
  he3->set_vertex (v_br);     he3_t->set_vertex (v_tr);
  he4->set_vertex (v_bl);     he4_t->set_vertex (v_br);

  oc->set_face (in_f);
  ic->set_face (fict_face);

  he1->set_inner_ccb (ic);       he1_t->set_outer_ccb (oc);
  he2->set_inner_ccb (ic);       he2_t->set_outer_ccb (oc);
  he3->set_inner_ccb (ic);       he3_t->set_outer_ccb (oc);
  he4->set_inner_ccb (ic);       he4_t->set_outer_ccb (oc);

  // Assign the incident halfedges of the two fictitious vertices.
  v_bl->set_halfedge (he1_t);
  v_tl->set_halfedge (he2_t);
  v_tr->set_halfedge (he3_t);
  v_br->set_halfedge (he4_t);

  // Set the direction of the halfedges:
  he1->set_direction (LEFT_TO_RIGHT);
  he2->set_direction (LEFT_TO_RIGHT);
  he3->set_direction (RIGHT_TO_LEFT);
  he4->set_direction (RIGHT_TO_LEFT);

  // Set the inner component of the fictitious face.
  fict_face->add_inner_ccb (ic, he1);

  // Set the real unbounded face, in the interior of the bounding rectangle.
  in_f->add_outer_ccb (oc, he1_t);
  in_f->set_unbounded (true);

  // Mark that there are four vertices at infinity (the fictitious ones)
  // in the arrangement.
  n_inf_verts = 4;

  return;
}

//-----------------------------------------------------------------------------
// Check if the given vertex is associated with the given curve end.
//
template <class GeomTraits, class Dcel_>
bool Arr_unb_planar_topology_traits_2<GeomTraits, Dcel_>::are_equal
    (const Vertex *v,
     const X_monotone_curve_2& cv, Curve_end ind,
     Boundary_type bound_x, Boundary_type bound_y) const
{
  // Make the given curve end lies at infinity.
  CGAL_precondition (bound_x == MINUS_INFINITY || bound_x == PLUS_INFINITY ||
                     bound_y == MINUS_INFINITY || bound_y == PLUS_INFINITY);

  // In case the given boundary conditions do not match those of the given
  // vertex, v cannot represent the curve end.
  if (bound_x != v->boundary_in_x() || bound_y != v->boundary_in_y())
    return (false);

  // Compare the curve end with the vertex.
  Comparison_result     res;

  if (bound_x != NO_BOUNDARY)
  {
    // The curve end lies at x = +/- oo and so does v. Check if the curve
    // overlaps with the curve that currently induces v.
    Curve_end                  v_ind;
    const X_monotone_curve_2  *v_cv = _curve (v, v_ind);

    if (v_cv == NULL)
      return (v->boundary_in_x() == bound_x &&
              v->boundary_in_y() == bound_y);

    CGAL_assertion (v_ind == ind);
    res = this->traits->compare_y_at_x_2_object() (cv, *v_cv, v_ind);
  }
  else 
  {
    CGAL_assertion (bound_y != NO_BOUNDARY);

    // The curve end lies at y = +/- oo and so does v. Check if the curve
    // overlaps with the curve that currently induces v.
    Curve_end                  v_ind;
    const X_monotone_curve_2  *v_cv = _curve (v, v_ind);

    if (v_cv == NULL)
      return (v->boundary_in_x() == NO_BOUNDARY &&
              v->boundary_in_y() == bound_y);

    res = this->traits->compare_x_2_object() (cv, ind,
                                              *v_cv, v_ind);
  }

  return (res == EQUAL);
}

//-----------------------------------------------------------------------------
// Given a curve end with boundary conditions and a face that contains the
// interior of the curve, find a place for a boundary vertex that will
// represent the curve end along the face boundary.
//
template <class GeomTraits, class Dcel_>
CGAL::Object
Arr_unb_planar_topology_traits_2<GeomTraits, Dcel_>::place_boundary_vertex
    (Face *f,
     const X_monotone_curve_2& cv, Curve_end ind,
     Boundary_type bound_x, Boundary_type bound_y)
{
  // Make the given curve end lies at infinity (the only boundary type
  // allowed for planar topology).
  CGAL_precondition (bound_x == MINUS_INFINITY || bound_x == PLUS_INFINITY ||
                     bound_y == MINUS_INFINITY || bound_y == PLUS_INFINITY);
                     
  // Get a halfedge on the outer CCB of f and start traversing the CCB.
  Halfedge           *first = *(f->outer_ccbs_begin());
  Halfedge           *curr = first;
  bool                eq_source, eq_target;

  do
  {
    // Note we consider only fictitious halfedges and check whether they
    // contain the relevant curve end.
    if (curr->has_null_curve() &&
        _is_on_fictitious_edge (cv, ind,
                                bound_x, bound_y,
                                curr,
                                eq_source, eq_target))
    {
      CGAL_assertion (! eq_source && ! eq_target);
      return (CGAL::make_object (curr));
    }

    // Move to the next halfegde along the CCB.
    curr = curr->next();

  } while (curr != first);

  // If we reached here, we did not find a suitable halfegde, which should
  // never happen.
  CGAL_assertion (false);
  return CGAL::Object();
}

//-----------------------------------------------------------------------------
// Locate a DCEL feature that contains the given unbounded curve end.
//
template <class GeomTraits, class Dcel_>
CGAL::Object Arr_unb_planar_topology_traits_2<GeomTraits, Dcel_>::
locate_unbounded_curve_end (const X_monotone_curve_2& cv, Curve_end ind,
                            Boundary_type bound_x, Boundary_type bound_y)
{
  // Make the given curve end lies at infinity.
  CGAL_precondition (bound_x == MINUS_INFINITY || bound_x == PLUS_INFINITY ||
                     bound_y == MINUS_INFINITY || bound_y == PLUS_INFINITY);

  // Start traversing the inner CCB of the fictitious face and try to locate
  // a feature that contains the curve end.
  Halfedge   *first = *(fict_face->inner_ccbs_begin());
  Halfedge   *curr = first;
  bool        eq_source, eq_target;

  do
  {
    if (_is_on_fictitious_edge (cv, ind,
                                bound_x, bound_y,
                                curr,
                                eq_source, eq_target))
    {
      if (eq_source)
      {
        // cv's end coincides with the source vertex of the current
        // fictitious halfedge. This means that cv overlaps the curve that
        // is associated with the only non-fictitious halfedge incident to
        // this vertex. We therefore return a pointer to this halfedge.
        Halfedge     *he = curr->opposite()->next();

        CGAL_assertion (! he->has_null_curve());
        return (CGAL::make_object (he));
      }
      else if (eq_target)
      {
        // cv's end coincides with the target vertex of the current
        // fictitious halfedge. This means that cv overlaps the curve that
        // is associated with the only non-fictitious halfedge incident to
        // this vertex. We therefore return a pointer to this halfedge.
        Halfedge     *he = curr->opposite()->prev();

        CGAL_assertion (! he->has_null_curve());
        return (CGAL::make_object (he));
      }

      // The current ficitious edge contains cv's end in its interior.
      // Note we use curr's twin, whose incident face is a valid
      // unbounded face (whereas the incident face of curr is the fictitious
      // face).
      Face      *uf = curr->opposite()->outer_ccb()->face();

      CGAL_assertion (uf->is_unbounded() && ! uf->is_fictitious());
      return (CGAL::make_object (uf));
    }

    curr = curr->next();

  } while (curr != first);

  // We should never reach here.
  CGAL_assertion (false);
  return Object();
}

//-----------------------------------------------------------------------------
// Split a fictitious edge using the given vertex.
//
template <class GeomTraits, class Dcel_>
typename Arr_unb_planar_topology_traits_2<GeomTraits, Dcel_>::Halfedge*
Arr_unb_planar_topology_traits_2<GeomTraits, Dcel_>::split_fictitious_edge
    (Halfedge *e, Vertex *v)
{
  CGAL_precondition (v->boundary_in_x() == MINUS_INFINITY ||
                     v->boundary_in_x() == PLUS_INFINITY ||
                     v->boundary_in_y() == MINUS_INFINITY ||
                     v->boundary_in_y() == PLUS_INFINITY);

  // Increment the number of vertices at infinity.
  n_inf_verts++;

  // Get the split halfedge and its twin, and their incident faces.
  // Note that he1 lies on an outer boundary of an unbounded face, while
  // its twin he2 should lie on a hole (inner boundary) inside the fictitious
  // face.
  Halfedge       *he1 = e;
  Halfedge       *he2 = he1->opposite();
  
  CGAL_assertion (! he1->is_on_inner_ccb());
  Outer_ccb      *oc1 = he1->outer_ccb();
  
  CGAL_assertion (oc1->face()->is_unbounded());

  CGAL_assertion (he2->is_on_inner_ccb());
  Inner_ccb      *ic2 = he2->inner_ccb();

  CGAL_assertion (ic2->face() == fict_face);

  // Allocate a pair of new halfedges.
  Halfedge       *he3 = this->m_dcel.new_edge();
  Halfedge       *he4 = he3->opposite();

  // Connect the new halfedges:
  //
  //            he1      he3
  //         -------> ------->
  //       (.)      (.)v     (.)
  //         <------- <-------
  //            he2      he4
  //
  v->set_halfedge (he4);

  // Connect e3 between e1 and its successor.
  he3->set_next (he1->next());

  // Insert he4 between he2 and its predecessor.
  he2->prev()->set_next (he4);

  // Set the properties of the new halfedges.
  he3->set_outer_ccb (oc1);
  he3->set_vertex (he1->vertex());

  he4->set_vertex (v);
  he4->set_next (he2);

  he4->set_inner_ccb (ic2);

  if (he1->vertex()->halfedge() == he1)
    // If he1 is the incident halfedge to its target, he3 replaces it.
    he1->vertex()->set_halfedge (he3);

  // Update the properties of the twin halfedges we have just split.
  he1->set_next(he3);
  he1->set_vertex(v);

  // The direction of he3 is the same as he1's (and the direction of he4 is
  // the same as he2).
  he3->set_direction (he1->direction());

  // Return a pointer to one of the existing halfedge that is incident to the
  // split vertex.
  return (he1);
}

//-----------------------------------------------------------------------------
// Determine whether the given face is unbounded.
//
template <class GeomTraits, class Dcel_>
bool Arr_unb_planar_topology_traits_2<GeomTraits, Dcel_>::is_unbounded
    (const Face *f) const
{
  // Go over the outer CBB of the given face and look for fictitious halfedges.
  const Halfedge   *first = *(f->outer_ccbs_begin());
  const Halfedge   *curr = first;

  do
  {
    if (curr->has_null_curve())
      // Found a fictitious halfedge along the boundary: f is unbounded.
      return (true);
    
    curr = curr->next();
    
  } while (curr != first);

  // If we reached here, all halfedges along the face boundary are valid,
  // thus the face is bounded.
  return (false);
}

//-----------------------------------------------------------------------------
// Determine whether the given boundary vertex is redundant.
//
template <class GeomTraits, class Dcel_>
bool Arr_unb_planar_topology_traits_2<GeomTraits, Dcel_>::is_redundant
    (const Vertex *v) const
{
  CGAL_precondition (v != v_bl && v != v_tl && v != v_br && v != v_tr);

  // A boundary vertex is redundant if there it is of degree 2 and (there
  // is no valid edge incident to it).
  const Halfedge  *first_he = v->halfedge();
  const Halfedge  *next_he = first_he->next()->opposite();
      
  if (next_he->next()->opposite() == first_he)
  {
    CGAL_assertion (first_he->has_null_curve() && next_he->has_null_curve());
    return (true);
  }

  return (false);
}

//-----------------------------------------------------------------------------
// Erase the given redundant vertex.
//
template <class GeomTraits, class Dcel_>
typename Arr_unb_planar_topology_traits_2<GeomTraits, Dcel_>::Halfedge*
Arr_unb_planar_topology_traits_2<GeomTraits, Dcel_>::erase_redundant_vertex
    (Vertex *v)
{
  CGAL_precondition (is_redundant (v));

  // Assign pointers to the halfedges incident to v (make sure that there
  // are exactly teo pairs of fictitious halfedges), such that we have:
  //
  //            he1      he3
  //         -------> ------->
  //       (.)      (.)v     (.)
  //         <------- <-------
  //            he2      he4
  //
  Halfedge   *he1 = v->halfedge();
  Halfedge   *he2 = he1->opposite();
  Halfedge   *he3 = he1->next();
  Halfedge   *he4 = he3->opposite();

  CGAL_assertion (he1->has_null_curve() && he3->has_null_curve() &&
                  he4->next() == he2);

  // Keep pointers to the components that contain two halfedges he3 and he2,
  // pointing at the end vertices of the merged halfedge.
  Inner_ccb   *ic1 = (he3->is_on_inner_ccb()) ? he3->inner_ccb() : NULL;
  Outer_ccb   *oc1 = (ic1 == NULL) ? he3->outer_ccb() : NULL;
  Inner_ccb   *ic2 = (he4->is_on_inner_ccb()) ? he4->inner_ccb() : NULL;
  Outer_ccb   *oc2 = (ic2 == NULL) ? he4->outer_ccb() : NULL;

  // As he1 and he2 will evetually represent the merged edge, while he3 and he4
  // will be deleted, check if the deleted halfedges are represantatives of a
  // face boundary or a hole inside these faces. If so, replace he3 by he1 and
  // he4 by he2.
  if (ic1 != NULL && ic1->halfedge() == he3)
    ic1->set_halfedge (he1);
  else if (oc1 != NULL && oc1->halfedge() == he3)
    oc1->set_halfedge (he1);

  if (ic2 != NULL && ic2->halfedge() == he4)
    ic2->set_halfedge (he2);
  else if (oc2 != NULL && oc2->halfedge() == he4)
    oc2->set_halfedge (he2);

  // If he3 is the incident halfedge to its target, replace it by he1.
  if (he3->vertex()->halfedge() == he3)
    he3->vertex()->set_halfedge (he1);

  // Disconnect he3 and he4 from the edge list.
  CGAL_assertion (he3->next() != he4);

  he1->set_next (he3->next());
  he4->prev()->set_next (he2);

  // Set the properties of the merged edge.
  he1->set_vertex (he3->vertex());

  // Decrement the number of vertices at infinity (note we do not actually
  // free the vertex - the Arrangement_on_surface_2 class will do it).
  n_inf_verts--;

  // Delete the redundant halfedge pair.
  this->m_dcel.delete_edge (he3);

  return (he1);
}

//-----------------------------------------------------------------------------
// Compare the x-coordinates of a given vertex (which may lie at infinity) and
// the given point.
//
template <class GeomTraits, class Dcel_>
Comparison_result
Arr_unb_planar_topology_traits_2<GeomTraits, Dcel_>::compare_x
    (const Point_2& p, const Vertex* v) const
{
  // First check if the vertex v lies at x = -oo (then it is obviously smaller
  // than p), or at x = +oo (then it is obviously larger).
  const Boundary_type          inf_x = v->boundary_in_x();

  if (inf_x == MINUS_INFINITY)
    return (LARGER);
  else if (inf_x == PLUS_INFINITY)
    return (SMALLER);

  // Check if the vertex lies at y = +/- oo.
  const Boundary_type          inf_y = v->boundary_in_y();

  if (inf_y != NO_BOUNDARY)
  {
    // Compare the x-position of the vertical asymptote of the curve incident
    // to v with the x-coodinate of p.
    Curve_end                  v_ind;
    const X_monotone_curve_2  *v_cv = _curve (v, v_ind);
    
    CGAL_assertion (v_cv != NULL);
    return (this->traits->compare_x_2_object() (p, *v_cv, v_ind));
  }

  // In this case v represents a normal point, and we compare it with p.
  return (this->traits->compare_x_2_object() (p, v->point()));
}

//-----------------------------------------------------------------------------
// Compare the given vertex (which may lie at infinity) and the given point.
//
template <class GeomTraits, class Dcel_>
Comparison_result
Arr_unb_planar_topology_traits_2<GeomTraits, Dcel_>::compare_xy
    (const Point_2& p, const Vertex* v) const
{
  // First check if the vertex v lies at x = -oo (then it is obviously smaller
  // than p), or at x = +oo (then it is obviously larger).
  const Boundary_type          inf_x = v->boundary_in_x();

  if (inf_x == MINUS_INFINITY)
    return (LARGER);
  else if (inf_x == PLUS_INFINITY)
    return (SMALLER);

  // Check if the vertex lies at y = +/- oo.
  const Boundary_type          inf_y = v->boundary_in_y();

  if (inf_y != NO_BOUNDARY)
  {
    // Compare the x-position of the vertical asymptote of the curve incident
    // to v with the x-coodinate of p.
    Curve_end                  v_ind;
    const X_monotone_curve_2  *v_cv = _curve (v, v_ind);

    CGAL_assertion (v_cv != NULL);

    Comparison_result          res =
      this->traits->compare_x_2_object() (p, *v_cv, v_ind);

    if (res != EQUAL)
      return (res);

    // In case of equality, consider whether v lies at y = -oo or at y = +oo.
    if (inf_y == MINUS_INFINITY)
      return (LARGER);
    else
      return (SMALLER);
  }

  // In this case v represents a normal point, and we compare it with p.
  return (this->traits->compare_xy_2_object() (p, v->point()));
}

//-----------------------------------------------------------------------------
// Compare the relative y-position of the given point and the given edge
// (which may be fictitious).
//
template <class GeomTraits, class Dcel_>
Comparison_result
Arr_unb_planar_topology_traits_2<GeomTraits, Dcel_>::compare_y_at_x
    (const Point_2& p, const Halfedge* he) const
{
  // In case of a valid edge, just compare p to its associated curve.
  if (! he->has_null_curve())
    return (this->traits->compare_y_at_x_2_object() (p, he->curve()));

  // Otherwise, determine on which edge of the bounding rectangle does he lie.
  // Note this can be either the top edge or the bottom edge (and not the
  // left or the right edge), as p must lie in its x-range.
  CGAL_assertion ((he->vertex()->boundary_in_x() == NO_BOUNDARY) ||
                  (he->vertex()->boundary_in_x() != 
                   he->opposite()->vertex()->boundary_in_x()));
  CGAL_assertion ((he->vertex()->boundary_in_y() != NO_BOUNDARY) &&
                  (he->vertex()->boundary_in_y() == 
                   he->opposite()->vertex()->boundary_in_y()));

  if (he->vertex()->boundary_in_y() == MINUS_INFINITY)
    // he lies on the bottom edge, so p is obviously above it.
    return (LARGER);
  else
    // he lies on the top edge, so p is obviously below it.
    return (SMALLER);
}

//-----------------------------------------------------------------------------
// Get the curve associated with a boundary vertex.
//
template <class GeomTraits, class Dcel_>
const typename
Arr_unb_planar_topology_traits_2<GeomTraits, Dcel_>::X_monotone_curve_2* 
Arr_unb_planar_topology_traits_2<GeomTraits, Dcel_>::_curve
    (const Vertex *v,
     Curve_end& ind) const
{
  // Go over the incident halfedges of v until encountering the halfedge
  // associated with a valid curve (v should have three incident halfedges,
  // two of the are fictitious and one associated with a curve).
  const Halfedge         *he = v->halfedge();

  while (he->has_null_curve())
  {
    he = he->next()->opposite();

    if (he == v->halfedge())
      // No incident curve were found:
      return (NULL);
  }

  // The halfedge he is directed toward v, so if it is directed from left to
  // right, v represents the maximal end of cv, otherwise it represents its
  // minimal end.
  ind = (he->direction() == LEFT_TO_RIGHT) ? MAX_END : MIN_END;
  
  // Return the x-monotone curve.
  return &(he->curve());
}

//-----------------------------------------------------------------------------
// Check whether the given infinite curve end lies on the given fictitious
// halfedge.
//
template <class GeomTraits, class Dcel_>
bool 
Arr_unb_planar_topology_traits_2<GeomTraits, Dcel_>::_is_on_fictitious_edge
    (const X_monotone_curve_2& cv, Curve_end ind,
     Boundary_type bound_x, Boundary_type bound_y,
     const Halfedge *he,
     bool& eq_source, bool& eq_target)
{
  eq_source = false;
  eq_target = false;

  // Get the end-vertices of the edge.
  const Vertex      *v1 = he->opposite()->vertex();
  const Vertex      *v2 = he->vertex();
  Boundary_type      he_bound;
  Comparison_result  res1, res2;
  Curve_end          v_ind;

  // Check if this is a "vertical" ficitious edge.
  if ((he_bound = v1->boundary_in_x()) != NO_BOUNDARY &&
      he_bound == v2->boundary_in_x())
  {
    // If the edge lies on x = +/- oo, the curve endpoint must also lie there.
    CGAL_assertion (he_bound == MINUS_INFINITY || he_bound == PLUS_INFINITY);

    if (he_bound != bound_x)
      return (false);

    // Compare the y-position of the curve end to the source vertex.
    if (v1 == v_bl || v1 == v_br)
    {
      // These vertices are below any curve.
      res1 = LARGER;
    }
    else if (v1 == v_tl || v1 == v_tr)
    {
      // These vertices are above any curve.
      res1 = SMALLER;
    }
    else
    {
      const Curve_end  ind = (bound_x == MINUS_INFINITY) ? MIN_END : MAX_END;

      res1 = this->traits->compare_y_at_x_2_object() (cv,
                                                      *_curve (v1, v_ind),
                                                      ind);
      if (res1 == EQUAL)
      {
        eq_source = true;
        return (true);
      }
    }

    // Compare the y-position of the curve end to the target vertex.
    if (v2 == v_bl || v2 == v_br)
    {
      // These vertices are below any curve.
      res2 = LARGER;
    }
    else if (v2 == v_tl || v2 == v_tr)
    {
      // These vertices are above any curve.
      res2 = SMALLER;
    }
    else
    {
      const Curve_end  ind = (bound_x == MINUS_INFINITY) ? MIN_END : MAX_END;

      res2 = this->traits->compare_y_at_x_2_object() (cv,
                                                      *_curve (v2, v_ind),
                                                      ind);
      
      if (res2 == EQUAL)
      {
        eq_target = true;
        return (true);
      }
    }
  }
  else
  {
    // If we reched here, we have a "horizontal" fictitious halfedge.
    he_bound = v1->boundary_in_y();

    CGAL_assertion ((he_bound == MINUS_INFINITY ||
                     he_bound == PLUS_INFINITY) &&
                    he_bound == v2->boundary_in_y());

    // If the edge lies on y = +/- oo, the curve endpoint must also lie there
    // (and must not lies at x = +/- oo.
    if (bound_x != NO_BOUNDARY || he_bound != bound_y)
      return (false);

    // Compare the x-position of the curve end to the source vertex.
    if (v1 == v_bl || v1 == v_tl)
    {
      // These vertices are to the left of any curve.
      res1 = LARGER;
    }
    else if (v1 == v_br || v1 == v_tr)
    {
      // These vertices are to the right of any curve.
      res1 = SMALLER;
    }
    else
    {
      const X_monotone_curve_2  *v_cv1 = _curve (v1, v_ind);

      CGAL_assertion (v_cv1 != NULL);
      res1 = this->traits->compare_x_2_object() (cv, ind, *v_cv1, v_ind);
      
      if (res1 == EQUAL)
      {
        eq_source = true;
        return (true);
      }
    }

    // Compare the x-position of the curve end to the target vertex.
    if (v2 == v_bl || v2 == v_tl)
    {
      // These vertices are to the left of any curve.
      res2 = LARGER;
    }
    else if (v2 == v_br || v2 == v_tr)
    {
      // These vertices are to the right of any curve.
      res2 = SMALLER;
    }
    else
    {
      const X_monotone_curve_2  *v_cv2 = _curve (v2, v_ind);

      CGAL_assertion (v_cv2 != NULL);
      res2 = this->traits->compare_x_2_object() (cv, ind, *v_cv2, v_ind);

      if (res2 == EQUAL)
      {
        eq_target = true;
        return (true);
      }
    }
  }

  return (res1 != res2);
}

CGAL_END_NAMESPACE

#endif
