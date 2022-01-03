/*
 * Kangaroo Lib, the library of Kangaroo PFM
 * Copyright (C) 2015 Lucas Rioux-Maldague <lucasr.mal@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  US
 */

#include "model/commodity-manager.h"

namespace kangaroo::model {

const Commodity* CommodityManager::FindBySymbol(
    const std::string& symbol) const {
  return symbol_index_.FindOne(symbol);
}

absl::Status CommodityManager::ValidateInsert(
    const Commodity& commodity) const {
  if (!commodity.has_decimal_places() || commodity.decimal_places() < 0 ||
      commodity.decimal_places() > CommodityManager::kMaxDecimalPlaces) {
    return absl::InvalidArgumentError(
        "Decimal places must be between 0 and 6.");
  }
  if (commodity.has_currency()) {
    if (commodity.symbol().size() != 3) {
      return absl::InvalidArgumentError("Currency symbol size must be 3.");
    }
  } else if (commodity.has_security()) {
    if (!commodity.security().has_traded_currency_id()) {
      return absl::InvalidArgumentError(
          "Security must have a traded currency.");
    }
    if (!Security::Type_IsValid(commodity.security().type())) {
      return absl::InvalidArgumentError("Invalid security type.");
    }
    if (const Commodity* curr =
            Get(commodity.security().has_traded_currency_id());
        curr == nullptr || !curr->has_currency()) {
      return absl::InvalidArgumentError(
          "Security traded currency must exist and be a currency.");
    }
    if (commodity.security().has_icon_id()) {
      return absl::UnimplementedError("Must Check This!");
    }
    if (commodity.security().has_provider_institution_id() &&
        !institution_manager_->Get(
            commodity.security().has_provider_institution_id())) {
      return absl::InvalidArgumentError("Provider instition does not exist.");
    }

  } else {
    return absl::InvalidArgumentError(
        "Commodity must be one of Currency or Security.");
  }

  return absl::OkStatus();
}

absl::Status CommodityManager::ValidateUpdate(const Commodity& before,
                                              const Commodity& after) const {
  if ((before.has_currency() && !after.has_currency()) ||
      (before.has_security() && !after.has_security())) {
    return absl::InvalidArgumentError("Commodity type may not be changed.");
  }
  return ValidateInsert(after);
}

absl::Status CommodityManager::ValidateRemove(
    const Commodity& commodity) const {
  if (commodity.has_currency()) {
    // Check for all security with this currency id.
    for (auto it = objects_.iterator(); it != objects_.iterator_end(); ++it) {
      if (it->second->has_security() &&
          it->second->security().traded_currency_id() == commodity.id()) {
        return absl::InvalidArgumentError(
            "Cannot delete currency that is referenced by at least one "
            "security.");
      }
    }
  }

  // TODO(lucasrmal): Check all accounts with this commodity id
  return absl::UnimplementedError("Not Yet Implemented!");

  // // Remove the price pair
  // PriceManager::instance()->removeAll(c->code());
}

// bool canRemove_recursiveA(const std::string& _code, Account* _a) {
//   if (!_a) {
//     return true;
//   } else if (_a->mainCurrency() == _code) {
//     return false;
//   } else {
//     for (Account* c : _a->getChildren()) {
//       if (!canRemove_recursiveA(_code, c)) return false;
//     }
//   }

//   return true;
// }

// bool canDeleteS(const std::string& _code) {
//   for (Security* s : SecurityManager::instance()->securities()) {
//     if (s->currency() == _code) return false;
//   }

//   return true;
// }

const std::vector<WorldCurrency>& CommodityManager::WorldCurrencies() {
  static const std::vector<WorldCurrency> currencies = {
      {"Albanian lek", "ALL", "L"},
      {"Afghan afghani", "AFN", "؋"},
      {"Algerian dinar", "DZN", "د.ج"},
      {"Angolan kwanza", "AOA", "Kz"},
      {"Argentine peso", "ARS", "$"},
      {"Armenian dram", "AMD", ""},
      {"Aruban florin", "AWG", "ƒ"},
      {"Australian dollar", "AUD", "$"},
      {"Azerbaijani manat", "AZN", ""},
      {"Bahamian dollar", "BDS", "$"},
      {"Bahraini dinar", "BHD", "", 3},
      {"Bangladeshi taka", "BDT", "৳"},
      {"Barbadian dollar", "BBD", "$"},
      {"Belarusian ruble", "BYR", "Br"},
      {"Belize dollar", "BZD", "$"},
      {"Bermudian dollar", "BMD", "$"},
      {"Bhutanese ngultrum", "BTN", "Nu."},
      {"Bolivian boliviano", "BOB", "Bs."},
      {"Bosnia and Herzegovina convertible mark", "BAM", "KM"},
      {"Botswana pula", "BWP", "P"},
      {"Brazilian real", "BRL", "R$"},
      {"British Pound", "GBP", "£"},
      {"Brunei dollar", "BND", "$"},
      {"Bulgarian lev", "BGN", "лв"},
      {"Burmese kyat", "MKK", "Ks"},
      {"Burundian franc", "BIF", "Fr"},
      {"Cambodian riel", "KHR", "៛"},
      {"Canadian Dollar", "CAD", "$"},
      {"Cape Verdean escudo", "CVE", "Esc"},
      {"Cayman Islands dollar", "KYD", "$"},
      {"Central African CFA franc", "XAF", "Fr"},
      {"CFP franc", "XPF", "Fr"},
      {"Chilean peso", "CLP", "$"},
      {"Chinese yuan", "CNY", "¥"},
      {"Colombian peso", "COP", "$"},
      {"Comorian franc", "KMF", "Fr"},
      {"Congolese franc", "CFD", "Fr"},
      {"Costa Rican colón", "CRC", "₡"},
      {"Croatian kuna", "HRK", "kn"},
      {"Cuban convertible peso", "CUC", "$"},
      {"Cuban peso", "CUP", "$"},
      {"Czech koruna", "CZK", "Kč"},
      {"Danish krone", "DKK", "kr"},
      {"Djiboutian franc", "DJF", "Fr"},
      {"Dominican peso", "DOP", "$"},
      {"East Caribbean dollar", "XCD", "$"},
      {"Egyptian pound", "EGP", "£"},
      {"Eritrean nakfa", "ERN", "Nfk"},
      {"Ethiopian birr", "ETB", "Br"},
      {"Euro", "EUR", "€"},
      {"Falkland Islands pound", "FKP", "£"},
      {"Fijian dollar", "FJD", "$"},
      {"Gambian dalasi", "GMD", "D"},
      {"Georgian lari", "GEL", "ლ"},
      {"Ghana cedi", "GHS", "₵"},
      {"Gibraltar pound", "GIP", "£"},
      {"Guatemalan quetzal", "GTQ", "Q"},
      {"Guernsey pound", "GGP", "£"},
      {"Guinean franc", "GNF", "Fr"},
      {"Guyanese dollar", "GYD", "$"},
      {"Haitian gourde", "HTG", "G"},
      {"Honduran lempira", "HNL", "L"},
      {"Hong Kong dollar", "HKD", "$"},
      {"Hungarian forint", "HUF", "Ft"},
      {"Icelandic króna", "ISK", "kr"},
      {"Indian rupee", "INR", "₹"},
      {"Indonesian rupiah", "IDR", "Rp"},
      {"Iranian rial", "IRR", "﷼"},
      {"Iraqi dinar", "IQD", "", 3},
      {"Israeli new shekel", "ILS", "₪"},
      {"Jamaican dollar", "JMD", "$"},
      {"Japanese yen", "JPY", "¥"},
      {"Jersey pound", "JEP", "£"},
      {"Jordanian dinar", "JOD", "د.ا"},
      {"Kazakhstani tenge", "KZT", "₸"},
      {"Kenyan shilling", "KES", "Sh"},
      {"Kuwaiti dinar", "KWD", "د.ك"},
      {"Kyrgyzstani som", "KGS", "лв"},
      {"Lao kip", "LAK", "₭"},
      {"Lebanese pound", "LBP", ""},
      {"Lesotho loti", "LSL", "L"},
      {"Liberian dollar", "LRD", "$"},
      {"Libyan dinar", "LYD", ""},
      {"Lithuanian litas", "LTL", "Lt"},
      {"Macanese pataca", "MOP", "P"},
      {"Macedonian denar", "MKD", "ден"},
      {"Malagasy ariary", "MGA", "Ar"},
      {"Malawian kwacha", "MWK", "MK"},
      {"Malaysian ringgit", "MYR", "RM"},
      {"Maldivian rufiyaa", "MVR", ""},
      {"Manx pound", "IMP", "£"},
      {"Mauritanian ouguiya", "MRO", "UM"},
      {"Mauritian rupee", "MUR", "Rs"},
      {"Mexican peso", "MXN", "$"},
      {"Moldovan leu", "MDL", "L"},
      {"Mongolian tögrög", "MNT", "₮"},
      {"Moroccan dirham", "MAD", "د.م."},
      {"Mozambican metical", "MZN", "MT"},
      {"Namibian dollar", "NAD", "$"},
      {"Nepalese rupee", "NPR", "Rs"},
      {"Netherlands Antillean guilder", "ANG", "ƒ"},
      {"New Taiwan dollar", "TWD", "$"},
      {"New Zealand dollar", "NZD", "$"},
      {"Nicaraguan córdoba", "NIO", "C$"},
      {"Nigerian naira", "NGN", "₦"},
      {"North Korean won", "KPW", "₩"},
      {"Norwegian krone", "NOK", "kr"},
      {"Omani rial", "OMR", "ر.ع."},
      {"Pakistani rupee", "PKR", "Rs"},
      {"Panamanian balboa", "PAB", "B/."},
      {"Papua New Guinean kina", "PGK", "K"},
      {"Paraguayan guaraní", "PYG", "₲"},
      {"Peruvian nuevo sol", "PEN", "S/."},
      {"Philippine peso", "PHP", "₱"},
      {"Polish złoty", "PLN", "zł"},
      {"Qatari riyal", "QAR", "ر.ق"},
      {"Romanian leu", "RON", "lei"},
      {"Russian ruble", "RUB", "р."},
      {"Rwandan franc", "RWF", "Fr"},
      {"Saint Helena pound", "SHP", "£"},
      {"Samoan tālā", "WST", "T"},
      {"Saudi riyal", "SAR", "ر.س"},
      {"Serbian dinar", "RSD", "дин"},
      {"Seychellois rupee", "SCR", "Rs"},
      {"Sierra Leonean leone", "SLL", "Le"},
      {"Singapore dollar", "SGD", "$"},
      {"Solomon Islands dollar", "SBD", "$"},
      {"Somali shilling", "SOS", "Sh"},
      {"South African rand", "ZAR", "R"},
      {"South Korean won", "KRW", "₩"},
      {"South Sudanese pound", "SSP", "£"},
      {"Sri Lankan rupee", "LKR", "Rs"},
      {"Sudanese pound", "SDG", "£"},
      {"Surinamese dollar", "SRD", "$"},
      {"Swazi lilangeni", "SZL", "L"},
      {"Swedish krona", "SEK", "kr"},
      {"Swiss franc", "CHF", "Fr"},
      {"Syrian pound", "SYP", "£"},
      {"São Tomé and Príncipe dobra", "STD", "Db"},
      {"Tajikistani somoni", "TJS", "SM"},
      {"Tanzanian shilling", "TZS", "Sh"},
      {"Thai baht", "THB", "฿"},
      {"Tongan paʻanga", "TOP", "T$"},
      {"Transnistrian ruble", "PRB", "p."},
      {"Trinidad and Tobago dollar", "TTD", "$"},
      {"Tunisian dinar", "TND", "د.ت"},
      {"Turkish lira", "TRY", ""},
      {"Turkmenistan manat", "TMT", "m"},
      {"Ugandan shilling", "UGX", "Sh"},
      {"Ukrainian hryvnia", "UAH", "₴"},
      {"United Arab Emirates dirham", "UAE", "د.إ"},
      {"United States Dollar", "USD", "$"},
      {"Uruguayan peso", "UYU", "$"},
      {"Uzbekistani som", "UZS", "лв"},
      {"Vanuatu vatu", "VUV", "Vt"},
      {"Venezuelan bolívar", "VEF", "Bs F"},
      {"Vietnamese đồng", "VND", "₫"},
      {"West African CFA franc", "XOF", "Fr"},
      {"Yemeni rial", "YER", "﷼"},
      {"Zambian kwacha", "ZMV", "ZK"}};
  return currencies;
}

}  // namespace kangaroo::model
