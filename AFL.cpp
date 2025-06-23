// Example fuzzing harness for QuantLib option pricing
#include <ql/quantlib.hpp>
#include <iostream>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size < sizeof(double) * 4) return 0;
    
    // Parse fuzzer input into option parameters
    double spot = *reinterpret_cast<const double*>(data);
    double strike = *reinterpret_cast<const double*>(data + sizeof(double));
    double rate = *reinterpret_cast<const double*>(data + 2 * sizeof(double));
    double volatility = *reinterpret_cast<const double*>(data + 3 * sizeof(double));
    
    try {
        using namespace QuantLib;
        
        // Basic validation
        if (spot <= 0 || strike <= 0 || volatility < 0) return 0;
        
        // Create option and calculate price
        Date today = Date::todaysDate();
        Date expiry = today + 30; // 30 days
        
        auto exercise = std::make_shared<EuropeanExercise>(expiry);
        auto payoff = std::make_shared<PlainVanillaPayoff>(Option::Call, strike);
        
        VanillaOption option(payoff, exercise);
        
        // Set up market data
        Handle<Quote> underlyingH(std::make_shared<SimpleQuote>(spot));
        Handle<YieldTermStructure> flatTermStructure(
            std::make_shared<FlatForward>(today, rate, Actual365Fixed()));
        Handle<BlackVolTermStructure> flatVolTS(
            std::make_shared<BlackConstantVol>(today, TARGET(), volatility, Actual365Fixed()));
        
        auto bsmProcess = std::make_shared<BlackScholesMertonProcess>(
            underlyingH, flatTermStructure, flatTermStructure, flatVolTS);
        
        option.setPricingEngine(std::make_shared<AnalyticEuropeanEngine>(bsmProcess));
        
        // This is what we're testing - should not crash
        double price = option.NPV();
        
    } catch (const std::exception& e) {
        // Expected for invalid inputs
        return 0;
    }
    
    return 0;
}