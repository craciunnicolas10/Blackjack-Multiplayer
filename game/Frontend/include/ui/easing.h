#pragma once

// Curbe de animație: primesc progresul `t` între 0 (început) și 1 (sfârșit) și întorc cât din
// drum s-a parcurs. Fac mișcările să pară naturale în loc de liniare.
namespace Frontend {
    // Pornește repede și încetinește la final, ca o carte aruncată pe masă.
    float easeOutCubic(float t);

    // Pornește încet, accelerează și încetinește iar la final (jetoanele care zboară).
    float easeInOutCubic(float t);

    // Depășește puțin 1 și revine, ca o etichetă care „sare” la locul ei. Cu cât
    // `overshoot` e mai mare, cu atât sare mai tare.
    float easeOutBack(float t, float overshoot);
}  // namespace Frontend
