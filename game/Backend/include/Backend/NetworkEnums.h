#pragma once

namespace Backend {
    // Rezultatul incercarii de a intra intr-un joc in retea (INetworkClient::Produce).
    enum class EConnectResult {
        Connected,
        InvalidName,     // gol sau prea lung
        InvalidAddress,  // adresa gazdei nu e buna
        Unreachable      // gazda nu raspunde (adresa gresita, jocul nu e creat, firewall)
    };

    // De ce a refuzat gazda intrarea in joc (INetworkListener::OnJoinRejected).
    enum class EJoinRejection {
        DifferentVersion,  // gazda are alta versiune a jocului
        TableFull,
        InvalidName,
        NameTaken
    };
}  // namespace Backend
