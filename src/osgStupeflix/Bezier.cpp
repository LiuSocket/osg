#include <osgStupeflix/Bezier>
/*  -*-c++-*-
 *  Copyright (C) 2010 Stupeflix
 *
 * Authors:
 *         Cedric Pinson <cedric.pinson@plopbyte.net>
 */


#include <osgStupeflix/Bezier>


using namespace osgStupeflix;

#define PI M_PI

static double cubic_root(double d) {
    if (d < 0) {
        return -pow(-d, 1.0 / 3.0);
    } else {
        return pow(d, 1.0 / 3.0);
    }
}

ReparametrizedBezierInterpolator::ReparametrizedBezierInterpolator(osg::Vec2 p0, osg::Vec2 p1, osg::Vec2 p2, osg::Vec2 p3):
  CubicBezierInterpolator<osg::Vec2>(p0, p1, p2, p3)
{
    init(p0,p1,p2,p3);
}

void ReparametrizedBezierInterpolator::init(osg::Vec2 p0, osg::Vec2 p1, osg::Vec2 p2, osg::Vec2 p3)
{
    _p0 = p0;
    _p1 = p1;
    _p2 = p2;
    _p3 = p3;

    double t0 = p0[0];
    double t1 = p1[0];
    double t2 = p2[0];
    double t3 = p3[0];

    _t0 = t0;
    _t1 = t1;
    _t2 = t2;
    _t3 = t3;

    // time = (1 - l) ^ 3  * t0 + 3 * (1 - l) ^ 2 * l * t1 + 3 * (1 - l) * l ^ 2 * t2 + l^ 3 * t3
    // =>
    // time = -l^3*t0 + 3*l^3*t1 - 3*l^3*t2 + l^3*t3 + 3*l^2*t0 - 6*l^2*t1 + 3*l^2*t2 - 3*l*t0 + 3*l*t1 + t0

    double a3 = -t0 + 3*t1 - 3*t2 + t3;
    double a2 = 3*t0 - 6*t1 + 3*t2;
    double a1 = - 3*t0 + 3*t1;
    double a0 = t0;

    _a3 = a3;
    _a2 = a2;
    _a1 = a1;
    _a0 = a0;

    //osg::notify(osg::INFO) << "ReparametrizedBezierInterpolator: a = " << a3 << " " << a2 << " " << a1 << " " << a0 << ", " << p0  << ", " << p1 << ", " << p2 << ", " << p3 << std::endl;

    // # From http://en.wikipedia.org/wiki/Cubic_function
    // # Q = math.sqrt((2 * a2^3 - 9 * a3 * a2 * a1 + 27 * a3^2 * a0) ^ 2 - 4 * (a2^2 - 3 * a3 * a1) ^ 3)
    _q0 = 2  * pow(a2, 3) - 9 * a3 * a2 * a1;
    _q1 = 27 * pow(a3, 2.0);
    _q2 = 4  * pow((pow(a2, 2.0) - 3 * a3 * a1), 3.0);

    // q4 = self.q0 + self.q1 * a0 (contains 10 which varies with the input time)
    // => Q = math.sqrt(pow(q0 + q1 * a0, 2) - q2)
    // => Q = math.sqrt(pow(q4,2) - q2)

    // C = math.pow(1.0/2*(Q + 2 * a2^3 - 9 * a3 * a2 * a1 + 27 * a3 ^ 2 * a0), 1.0/ 3)
    // C = math.pow(1.0/2*(Q + q0 + self.q1 * a0), 1.0/ 3)
    // C = math.pow(1.0/2*(Q + q4), 1.0/ 3)

    _f0 =  - 1.0 / (3 * a3);
    _f1 = pow(a2,2) - 3 * a3 * a1;

    // x1 = - (a1 + C + (a2 ^ 2 - 3 * a3 * a1) / C) / (3 * a3)
    // x1 = f0 * (a2 + C + (a2 ^ 2 - 3 * a3 * a1) / C)
    // x1 = f0 * (a2 + C + f1 / C)
    //    printf("ReparametrizedBezierInterpolator %f, %f, %f, %f, %f, %f, %f, %f, %f\n", _a0, _a1, _a2, _a3, _q0, _q1, _q2, _f0, _f1);
}

double ReparametrizedBezierInterpolator::angle(double re, double im) const {
    if (re > 0) {
        return atan(im / re);
    }
    if (re < 0 && im >= 0) {
        return atan(im /re) + PI;
    }
    if (re < 0 && im < 0) {
        return atan(im /re) - PI;
    }
    if (re == 0 && im > 0) {
        return PI / 2;
    }
    if (re == 0 && im < 0) {
        return - PI / 2;
    }
    return 0.0;
}

#define SQRT_3_DIV_2 .86602540378443864676


double ReparametrizedBezierInterpolator::cubic_solve(double time) const {
    double a0 = _a0 - time;

    if (fabs(_a3) < 1.0E-6) {

        if (fabs(_a2) < 1.0E-6) {
            //            osg::notify(osg::INFO) << "linear_solve" << std::endl;
            return - a0 / _a1;
        } else {
            //            osg::notify(osg::INFO) << "quadratic_solve time= "<< time << " " << _a2 << " " << _a1 << " " << a0  << " " << _a0 << std::endl;
            double det = _a1 * _a1 - 4 * _a2 * a0;
            //            osg::notify(osg::INFO) << "det" << det << std::endl;
            if (det > 0) {
                det = sqrt(det);
                double x0 = (- _a1 - det) / (2 * _a2);
                double x1 = (- _a1 + det) / (2 * _a2);

                //                osg::notify(osg::INFO) << "x0 = " << x0 << " x1 = " << x1 << std::endl;

                if (x0 >= 0.0 && x0 <= 1.0) {
                    return x0;
                }
                if (x1 >= 0.0 && x1 <= 1.0) {
                    return x1;
                }
                //                osg::notify(osg::INFO) << "this should not happen 1" << std::endl;
                return 0.0;
            } else {
                //                osg::notify(osg::INFO) << "this should not happen 2" << std::endl;
                return 0.0;
            }
        }
    }

    //    osg::notify(osg::INFO) << "cubic_solve: " << _a3 << " " << _a2 << " " << _a1 << " " << a0 << std::endl;
    double q4 = _q0 + _q1 * a0;

    double dis = pow(q4, 2) - _q2;

    if (dis > 0) {
        //        osg::notify(osg::INFO) << "dis > 0" << std::endl;
        double Q = sqrt(dis);

        double c = 0.5 * (Q + q4);

        if (c == 0) {
            return cubic_root(-a0 / _a3);
        }

        double C = cubic_root(c);

        double l = _f0 * (_a2 + C + _f1 / C);


        //        osg::notify(osg::INFO) << "c= " << c << " C = " << C << " _f0=" << _f0 << std::endl;

        return l;
    } else {
        //        osg::notify(osg::INFO) << "dis < 0" << dis << std::endl;
        double Q_im = sqrt(-dis);
        double c_re = 0.5 * q4;
        double c_im = 0.5 * Q_im;

        double cnorm = sqrt(c_re * c_re + c_im * c_im);

        if (cnorm == 0) {
            return cubic_root(-a0 / _a3);
        }

        double cangle = angle(c_re, c_im);

        double Cnorm = cubic_root(cnorm);

        double Cangle = cangle / 3.0;

        double cosCangle = cos(Cangle);
        double sinCangle = sin(Cangle);

        double C_re = Cnorm * cosCangle;
        double C_im = Cnorm * sinCangle;


        //        osg::notify(osg::INFO) << "C = (" << C_re  << "," << C_im << ")" << std::endl;

        // double C_inv_re = 1.0 / Cnorm * cos(-Cangle);
        // double C_inv_im = 1.0 / Cnorm * sin(-Cangle);
        double invCnorm = 1.0 / Cnorm;
        double C_inv_re = invCnorm  * cosCangle;
        double C_inv_im = - invCnorm  * sinCangle;

        int i;

        for (i = 0 ; i < 3; i++) {
            double l0_re =  _f0 * (_a2 + C_re + _f1 * C_inv_re);
            double l = l0_re;

            //            osg::notify(osg::INFO) << "l = " << l << std::endl;

            if (l >= (_t0 - 0.00001) and l <= (_t3 + 0.00001)) {
                return l;
            }

            // Equivalent to : Cangle += PI * 2 / 3;

            double C_re_new = - C_re * 0.5 - C_im * SQRT_3_DIV_2;
            double C_im_new = C_re * SQRT_3_DIV_2 - C_im * 0.5;

            C_re = C_re_new;
            C_im = C_im_new;

            double C_inv_re_new = - C_inv_re * 0.5 + C_inv_im * SQRT_3_DIV_2;
            double C_inv_im_new = -C_inv_re * SQRT_3_DIV_2 - C_inv_im * 0.5;

            C_inv_re = C_inv_re_new;
            C_inv_im = C_inv_im_new;

            // C_re = Cnorm * cos(Cangle);
            // C_im = Cnorm * sin(Cangle);

            // C_inv_re = 1.0 / Cnorm * cos(-Cangle);
            // C_inv_im = 1.0 / Cnorm * sin(-Cangle);
        }
    }
    return 0.0;
}


osg::Vec2 ReparametrizedBezierInterpolator::interpolate(double l) const {
    double l_ = cubic_solve(l);
    double v = interpolate_(l_)[1];
    return osg::Vec2(l, v);
}


