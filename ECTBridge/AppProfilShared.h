// AppProfilShared.h -- Zugriff von ECTViews.FensterPosition auf das
// App-Profil (theApp.Get/WriteProfileInt, Sektion [Fenster]).
//
// Definition in FormularExports.cpp. Haengt zwei Delegates an
// ECTViews::FensterPosition; mehrfacher Aufruf ist harmlos (nur der erste
// wirkt). Jede Ansicht, aus der ein WPF-Fenster mit gemerkter Position
// aufgehen kann, ruft sie beim Einbetten -- ohne Registrierung oeffnen
// diese Fenster einfach an ihrer Standardposition.
//
// Nur in /clr-Uebersetzungseinheiten aufrufen (die Definition ist managed).

#pragma once

void ECTBridge_RegistriereAppProfil();
