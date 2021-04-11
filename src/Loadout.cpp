#include "Loadout.h"

const array<string, 28> SLOT = {"BODY",
                                "DECAL",
                                "WHEELS",
                                "BOOST",
                                "ANTENNA",
                                "TOPPER",
                                "6",
                                "PRIMARY_FINISH",
                                "BOT",
                                "LOGO",
                                "10",
                                "PREMIUM_INVENTORY",
                                "ACCENT_FINISH",
                                "ENGINE",
                                "TRAIL",
                                "EXPLOSION",
                                "BANNER",
                                "17",
                                "ANTHEM",
                                "19",
                                "AVATAR_BORDER",
                                "TITLE",
                                "ESPORTS_TEAM",
                                "23",
                                "BLUEPRINT",
                                "SHOP_ITEM",
                                "CURRENCY",
                                "27"};

const array<string, 10> QUALITY = {
    "COMMON", "UNCOMMON",     "RARE",    "VERY_RARE", "IMPORT",
    "EXOTIC", "BLACK_MARKET", "PREMIUM", "LIMITED",   "LEGACY"};

const array<string, 19> PAINT = {
    "NONE",      "CRIMSON",      "LIME",           "BLACK",   "SKY_BLUE",
    "COBALT",    "BURNT_SIENNA", "FOREST_GREEN",   "PURPLE",  "PINK",
    "ORANGE",    "GREY",         "TITANIUM_WHITE", "SAFFRON", "GOLD",
    "ROSE_GOLD", "WHITE_GOLD",   "ONYX",           "PLATINUM"};

void Loadout::load(int teamNum, CVarManager cv, Game gw) {}

void Loadout::loadVanilla(int teamNum, CVarManager cv, Game gw) {
  ItemsWrapper iw = gw->GetItemsWrapper();
  if (iw.IsNull()) return cv->log("Unable to load Rocket League Loadout");

  LoadoutWrapper lw = iw.GetCurrentLoadout(teamNum);
  // TODO: Remove below when BakkesMod updates
  lw = LoadoutWrapper(*reinterpret_cast<uintptr_t*>(lw.memory_address));
  ArrayWrapper<uint64_t> items = lw.GetOnlineLoadout();

  bool foundPrimary = false;

  for (uint64_t id : items) {
    if (id == 0) continue;  // 0 indicates no data for that slot

    OnlineProductWrapper onlineProduct = iw.GetOnlineProduct(id);
    ProductWrapper* product = nullptr;

    // If the id does not match an online product, try checking normal
    // (local?) product list.
    if (onlineProduct.IsNull()) {
      // Check if casting unsigned long long (uint64) -> int is valid
      if (id - INT_MIN <= (uint64_t)INT_MAX - INT_MIN) {
        ProductWrapper prod = iw.GetProduct((int)id);
        product = &prod;
      } else {
        cv->log("Missing Online ID: " + to_string(id));
        continue;
      }
    } else {
      ProductWrapper prod = onlineProduct.GetProduct();
      product = &prod;
    }

    if (product == nullptr || product->IsNull()) {
      cv->log("ERROR: Product is null!");
      continue;
    }

    ProductSlotWrapper productSlot = product->GetSlot();
    if (productSlot.IsNull()) {
      cv->log("ERROR: Slot is null!");
      continue;
    }

    int slotIdx = productSlot.GetSlotIndex();

    // Assign second "finish" slot to accent/secondary finish
    if (slotIdx == 7) {
      if (foundPrimary) {
        slotIdx = 12;
      } else {
        foundPrimary = true;
      }
    }

    if (slotIdx < 0 || slotIdx >= SLOT.size()) {
      cv->log("Invalid Slot Index: " + to_string(slotIdx));
      continue;
    }

    stringstream ss;
    string slot = SLOT[slotIdx];
    string prodName = product->GetLongLabel().ToString();
    string rarity = QUALITY[product->GetQuality()];

    ss << SLOT[slotIdx] << ": " << prodName << " (";

    if (!!onlineProduct) {
      ArrayWrapper<ProductAttributeWrapper> attrs =
          onlineProduct.GetAttributes();
      if (!attrs.IsNull()) {
        for (ProductAttributeWrapper attr : attrs) {
          if (!attr.IsNull()) cv->log(attr.GetAttributeType());
        }
      } else {
        cv->log("Online Product Attributes were Null");
      }
    } else {
      cv->log("Online Product was Null");
    }
    if (!!product) {
      ArrayWrapper<ProductAttributeWrapper> attrs = product->GetAttributes();
      if (!attrs.IsNull()) {
        for (ProductAttributeWrapper attr : attrs) {
          if (!attr.IsNull()) cv->log(attr.GetAttributeType());
        }
      } else {
        cv->log("Product Attributes were Null");
      }
    } else {
      cv->log("Product was Null");
    }

    // for (ProductAttributeWrapper attr : product->GetAttributes()) {
    //   string type = attr.GetAttributeType();
    //   if (type == "ProductAttribute_Certified_TA") {
    //     string statName =
    //         gw->GetItemsWrapper().GetCertifiedStatDB().GetStatName(
    //             ProductAttribute_CertifiedWrapper(attr.memory_address)
    //                 .GetStatId());
    //     ss << " Certified=" << statName << " ";
    //   } else if (type == "ProductAttribute_Painted_TA") {
    //     string paint =
    //         PAINT[ProductAttribute_PaintedWrapper(attr.memory_address)
    //                   .GetPaintID()];
    //     ss << " Painted=" << paint << " ";
    //   } else if (type == "ProductAttribute_SpecialEdition_TA") {
    //     auto specEdName =
    //         gw->GetItemsWrapper().GetSpecialEditionDB().GetSpecialEditionName(
    //             ProductAttribute_SpecialEditionWrapper(attr.memory_address)
    //                 .GetEditionID());
    //     if (specEdName.find("Edition_") == 0) specEdName =
    //     specEdName.substr(8); ss << " SpecialEdition=" << specEdName << " ";
    //   } else if (type == "ProductAttribute_TeamEdition_TA") {
    //     string team = gw->GetItemsWrapper().GetEsportTeamDB().GetName(
    //         ProductAttribute_TeamEditionWrapper(attr.memory_address).GetId());
    //     ss << " TeamEdition=" << team << " ";
    //   }
    // }
    ss << ")";
    cv->log(ss.str());
  }
}

void Loadout::loadBakkes(int teamNum, CVarManager cv, Game gw) {
  if (!cv->getCvar("cl_itemmod_enabled").getBoolValue()) return;
  string itemCode = cv->getCvar("cl_itemmod_code").getStringValue();
  // BMCodeReader reader(itemCode);
}