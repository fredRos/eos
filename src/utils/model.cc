/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include <src/utils/model.hh>
#include <src/utils/private_implementation_pattern-impl.hh>
#include <src/utils/qcd.hh>

#include <cmath>

namespace wf
{
    using std::sqrt;

    Model::~Model()
    {
    }

    template <> struct Implementation<StandardModel>
    {
        /* CKM Wolfenstein parameters */
        Parameter A;
        Parameter lambda;
        Parameter rhobar;
        Parameter etabar;
        // Measurement of V_cb is quite good, so use that
        Parameter ckm_cb;

        /* QCD parameters */
        Parameter alpha_s_Z;
        Parameter mu_t;
        Parameter mu_b;
        Parameter mu_c;

        /* Masses */
        Parameter m_b_MSbar;
        Parameter m_c_MSbar;
        Parameter m_Z;

        /* Energy Scale */
        Parameter mu;

        Implementation(const Parameters & p) :
            A(p["CKM::A"]),
            lambda(p["CKM::lambda"]),
            rhobar(p["CKM::rhobar"]),
            etabar(p["CKM::etabar"]),
            ckm_cb(p["CKM::|V_cb|"]),
            alpha_s_Z(p["QCD::alpha_s(MZ)"]),
            mu_t(p["QCD::mu_t"]),
            mu_b(p["QCD::mu_b"]),
            mu_c(p["QCD::mu_c"]),
            m_b_MSbar(p["mass::b(MSbar)"]),
            m_c_MSbar(p["mass::c"]),
            m_Z(p["mass::Z"]),
            mu(p["mu"])
        {
        }

        static QCD::Parameters qcd_params_nf5;

        static QCD::Parameters qcd_params_nf4;
    };

    QCD::Parameters Implementation<StandardModel>::qcd_params_nf5 = QCD::Parameters()
                .nf(5.0)
                .a1(43.0 / 9.0)
                .a2(155.829)
                .beta0(23.0 / 3.0)
                .beta1(116.0 / 3.0)
                .beta2(9769.0 / 54.0)
                .beta3(4826.1563287908967)
                .gamma0_m(1.0)
                .gamma1_m(506.0 / 9.0)
                .gamma2_m(474.87124557719461)
                .gamma3_m(2824.7862379694232);

    QCD::Parameters Implementation<StandardModel>::qcd_params_nf4 = QCD::Parameters()
                .nf(4.0)
                .a1(53.0 / 9.0)
                .a2(211.077)
                .beta0(25.0 / 3.0)
                .beta1(154.0 / 3.0)
                .beta2(21943.0 / 54.0)
                .beta3(8035.1864197901160)
                .gamma0_m(1.0)
                .gamma1_m(526.0 / 9.0)
                .gamma2_m(636.61057670866927)
                .gamma3_m(6989.5510103599477);

    StandardModel::StandardModel(const Parameters & p) :
        PrivateImplementationPattern<StandardModel>(new Implementation<StandardModel>(p))
    {
    }

    StandardModel::~StandardModel()
    {
    }

    double
    StandardModel::alpha_s(const double & mu) const
    {
        if (mu > _imp->m_Z)
            throw InternalError("StandardModel::alpha_s: Running of alpha_s to mu > m_Z not yet implemented");

        double alpha_s_0 = _imp->alpha_s_Z, mu_0 = _imp->m_Z;

        if (mu >= _imp->mu_b)
            return QCD::alpha_s(mu, alpha_s_0, mu_0, Implementation<StandardModel>::qcd_params_nf5);

        alpha_s_0 = QCD::alpha_s(_imp->mu_b, alpha_s_0, mu_0, Implementation<StandardModel>::qcd_params_nf5);
        mu_0 = _imp->mu_b;

        if (mu >= _imp->mu_c)
            return QCD::alpha_s(mu, alpha_s_0, mu_0, Implementation<StandardModel>::qcd_params_nf4);

        throw InternalError("StandardModel::alpha_s: Running of alpha_s to mu < mu_c not yet implemented");
    }

    double
    StandardModel::m_b_msbar(const double & mu) const
    {
        double m_b_MSbar = _imp->m_b_MSbar();
        double alpha_mu_0 = alpha_s(m_b_MSbar);

        if (mu > _imp->m_b_MSbar)
        {
            if (mu < _imp->mu_t)
                return QCD::m_q_msbar(m_b_MSbar, alpha_mu_0, alpha_s(mu), Implementation<StandardModel>::qcd_params_nf5);

            throw InternalError("StandardModel::m_b_msbar: Running of m_b_MSbar to mu > mu_t not yet implemented");
        }
        else
        {
            if (mu >= _imp->mu_c)
                return QCD::m_q_msbar(m_b_MSbar, alpha_mu_0, alpha_s(mu), Implementation<StandardModel>::qcd_params_nf4);

            throw InternalError("StandardModel::m_b_msbar: Running of m_b_MSbar to mu < mu_c not yet implemented");
        }
    }

    double
    StandardModel::m_b_pole() const
    {
        double m_b_MSbar = _imp->m_b_MSbar();

        return QCD::m_q_pole(m_b_MSbar, alpha_s(m_b_MSbar), Implementation<StandardModel>::qcd_params_nf5);
    }

    double
    StandardModel::m_b_ps(const double & mu_f) const
    {
        double m_b_MSbar = _imp->m_b_MSbar();

        return QCD::m_q_ps(m_b_MSbar, alpha_s(m_b_MSbar), mu_f, Implementation<StandardModel>::qcd_params_nf5);
    }

    /* Charm */
    double
    StandardModel::m_c_msbar(const double & mu) const
    {
        double m_c_0 = _imp->m_c_MSbar();
        double alpha_s_mu0 = alpha_s(m_c_0);

        if (mu >= _imp->m_c_MSbar)
        {
            if (mu <= _imp->mu_b)
                return QCD::m_q_msbar(m_c_0, alpha_s_mu0, alpha_s(mu), Implementation<StandardModel>::qcd_params_nf4);

            double alpha_s_b = alpha_s(_imp->mu_b);
            m_c_0 = QCD::m_q_msbar(m_c_0, alpha_s_mu0, alpha_s_b, Implementation<StandardModel>::qcd_params_nf4);
            alpha_s_mu0 = alpha_s_b;

            if (mu <= _imp->mu_t)
                return QCD::m_q_msbar(m_c_0, alpha_s_mu0, alpha_s(mu), Implementation<StandardModel>::qcd_params_nf5);

            throw InternalError("StandardModel::m_c_msbar: Running of m_c_MSbar to mu > mu_t not yet implemented");
        }
        else
        {
            throw InternalError("StandardModel::m_c_msbar: Running of m_c_MSbar to mu < mu_c not yet implemented");
        }
    }

    double
    StandardModel::m_c_pole() const
    {
        double m_c_MSbar = _imp->m_c_MSbar();

        return QCD::m_q_pole(m_c_MSbar, alpha_s(m_c_MSbar), Implementation<StandardModel>::qcd_params_nf4);
    }

    complex<double>
    StandardModel::ckm_cb() const
    {
        return complex<double>(_imp->ckm_cb, 0.0);
    }

    complex<double>
    StandardModel::ckm_us() const
    {
        return complex<double>(_imp->lambda, 0.0);
    }

    complex<double>
    StandardModel::ckm_ub() const
    {
        double lambda2 = _imp->lambda * _imp->lambda, lambda3 = _imp->lambda * lambda2, lambda4 = lambda2 * lambda3;
        double A2 = _imp->A * _imp->A;
        complex<double> num = complex<double>(_imp->rhobar, -1.0 * _imp->etabar);
        complex<double> denom = 1.0 - num * A2 * lambda4;
        double rest =  _imp->A * lambda3 * sqrt((1.0 - A2 * lambda4) / (1.0 - lambda2));

        return (num / denom) * rest;
    }

    complex<double>
    StandardModel::ckm_ts() const
    {
        return complex<double>(-1.0 * _imp->A * _imp->lambda * _imp->lambda, 0.0);
    }

    complex<double>
    StandardModel::ckm_tb() const
    {
        return complex<double>(1.0, 0.0);
    }
}
