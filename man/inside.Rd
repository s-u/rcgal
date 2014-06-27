\name{inside}
\alias{inside}
\title{
  Finds shapes that contain given points
}
\description{
  \code{inside} returns indices of shapes that contain given points
}
\usage{
inside(shp, x, y)
}
\arguments{
  \item{shp}{shape object (as returned by \code{read.shp(...,
      format="polygon")}) to match against}
  \item{x}{x coordinates of the points to match}
  \item{y}{y coordinates of the points to match}
}
\details{
  The matching uses bounding box as a first approximation and then
  CGAL point/polygon test to determine if a point is inside a shape,
  returninig the first shape that matches or \code{NA} otherwise. 
  Holes are supported as well as multi-part polygons.
}
\value{
  If \code{all=FALSE}:
  Integer vector of the same length as the number of points, each value
  is either an index of the first matching shape or \code{NA} if the
  point is not inside of any shapes.

  If \code{all=TRUE}:
  List of integer vectors with the indices of matching shapes (which
  whill be empty if there is no match). There will be as many elements
  in the list as there are points.
}
%\references{
%}
\author{
  Simon Urbanek
}
%\seealso{
%}
%\examples{
%}
\keyword{manip}
